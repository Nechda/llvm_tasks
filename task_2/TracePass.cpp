#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#if __has_include("llvm/Plugins/PassPlugin.h") /* LLVM >= 24 */
#include "llvm/Plugins/PassPlugin.h"
#else
#include "llvm/Passes/PassPlugin.h"
#endif
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

using namespace llvm;

/*
 * Инструментирующий модульный pass: строит в рантайме трассу использования
 * IR инструкций вида (User <- Operand).
 *
 * Для каждой инструкции-User (кроме PHI узлов) перед инструкцией вставляются
 * вызовы void __trace_pair(i32 userId, i32 operandId) -- по одному на каждый
 * операнд. operandId -- это id инструкции-операнда, либо 0, если операнд не
 * является инструкцией (константа, аргумент, глобал и т.п.).
 * PHI узлы в качестве User пропускаются, но могут выступать операндами.
 *
 * Каждой инструкции модуля присваивается уникальный uint32 id, соответствие
 * id -> инструкция выгружается в map-файл (env LLVM_TRACE_MAP, по умолчанию
 * "trace_map.txt"): id \t opcode \t function \t text.
 */

namespace {

constexpr uint32_t kNonInstructionId = 0;

struct TraceInstrumentPass {
    static StringRef name() { return "trace-instr"; }
    static bool isRequired() { return true; }
    void
    printPipeline(raw_ostream &OS,
                  function_ref<StringRef(StringRef)> ClassName2PassName) const {
        OS << name();
    }

    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        LLVMContext &Ctx = M.getContext();

        // void __trace_pair(i32, i32)
        auto *TraceTy = FunctionType::get(
            Type::getVoidTy(Ctx),
            {Type::getInt32Ty(Ctx), Type::getInt32Ty(Ctx)}, false);
        FunctionCallee TraceCallee =
            M.getOrInsertFunction("__trace_pair", TraceTy);

        // 1. Назначить id всем инструкциям всех определённых функций.
        DenseMap<const Instruction *, uint32_t> Ids;
        std::vector<const Instruction *> IdToInst;
        IdToInst.push_back(nullptr); // id 0 -- зарезервирован (не инструкция)
        for (Function &F : M) {
            if (F.isDeclaration())
                continue;
            for (BasicBlock &BB : F)
                for (Instruction &I : BB) {
                    Ids[&I] = static_cast<uint32_t>(IdToInst.size());
                    IdToInst.push_back(&I);
                }
        }

        // 2. Выгрузить map-файл.
        std::string MapPath = [] {
            const char *Env = std::getenv("LLVM_TRACE_MAP");
            return Env ? std::string(Env) : std::string("trace_map.txt");
        }();
        {
            std::error_code EC;
            raw_fd_ostream Map(MapPath, EC, sys::fs::OF_TextWithCRLF);
            if (EC) {
                errs() << "[trace-instr] cannot open map file '" << MapPath
                       << "': " << EC.message() << "\n";
            } else {
                Map << "id\topcode\tfunction\tinstruction\n";
                for (size_t Id = 1; Id < IdToInst.size(); ++Id) {
                    const Instruction *I = IdToInst[Id];
                    std::string Text;
                    raw_string_ostream SS(Text);
                    I->print(SS);
                    Map << Id << '\t' << I->getOpcodeName() << '\t'
                        << I->getFunction()->getName() << '\t'
                        << StringRef(Text).trim() << '\n';
                }
            }
        }

        // 3. Инструментировать.
        uint64_t NumCallSites = 0; // instrumented user instructions
        uint64_t NumTraceCalls = 0;
        for (Function &F : M) {
            if (F.isDeclaration())
                continue;
            for (BasicBlock &BB : F) {
                for (auto It = BB.begin(); It != BB.end(); ++It) {
                    Instruction *User = &*It;

                    // User -- PHI: пропускаем по условию задачи.
                    if (isa<PHINode>(User))
                        continue;
                    // Не инструментируем сами трейсинговые вызовы.
                    if (auto *CB = dyn_cast<CallBase>(User)) {
                        if (CB->getCalledFunction() &&
                            CB->getCalledFunction()->getName() ==
                                "__trace_pair")
                            continue;
                    }

                    // Собираем пары (userId, operandId) до вставки вызовов.
                    SmallVector<uint32_t, 4> OpIds;
                    for (Value *Op : User->operand_values()) {
                        auto ItId = Ids.find(dyn_cast_or_null<Instruction>(Op));
                        OpIds.push_back(ItId != Ids.end() ? ItId->second
                                                          : kNonInstructionId);
                    }
                    if (OpIds.empty())
                        continue;

                    IRBuilder<> Builder(User);
                    for (uint32_t OpId : OpIds) {
                        Builder.CreateCall(
                            TraceCallee,
                            {ConstantInt::get(Type::getInt32Ty(Ctx), Ids[User]),
                             ConstantInt::get(Type::getInt32Ty(Ctx), OpId)});
                        ++NumTraceCalls;
                    }
                    ++NumCallSites;
                }
            }
        }

        outs() << "[trace-instr] module '" << M.getName()
               << "': " << (IdToInst.size() - 1) << " instructions mapped, "
               << NumCallSites << " users instrumented, " << NumTraceCalls
               << " trace calls inserted (map: " << MapPath << ")\n";
        return PreservedAnalyses::none();
    }
};

} // namespace

PassPluginLibraryInfo getPassPluginInfo() {
    const auto Callback = [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, ModulePassManager &MPM,
               ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "trace-instr") {
                    MPM.addPass(TraceInstrumentPass{});
                    return true;
                }
                return false;
            });
    };
    return {LLVM_PLUGIN_API_VERSION, "trace-instr", "0.0.1", Callback};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getPassPluginInfo();
}
