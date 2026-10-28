#include "app_ir_gen.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#ifdef TASK3_EXECUTE
#include "sim.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/DynamicLibrary.h"

using namespace llvm;

// Interpreter's external-call ABI. These adapters also work without libffi.
static GenericValue putPixelAdapter(FunctionType *,
                                    ArrayRef<GenericValue> args) {
    simPutPixel(static_cast<int>(args[0].IntVal.getSExtValue()),
                static_cast<int>(args[1].IntVal.getSExtValue()),
                static_cast<int>(args[2].IntVal.getSExtValue()));
    return {};
}

static GenericValue flushAdapter(FunctionType *, ArrayRef<GenericValue>) {
    simFlush();
    return {};
}

// Interpreter's IntrinsicLowering does not implement these intrinsics.
// Expand them only in the execution copy; the dumped module retains the calls.
// llvm.fmuladd permits separate multiply/add rounding (unlike llvm.fma).
static void prepareForInterpreter(Module &module) {
    for (auto &function : module) {
        for (auto &block : function) {
            for (auto it = block.begin(); it != block.end();) {
                auto *call = dyn_cast<IntrinsicInst>(&*it++);
                if (!call)
                    continue;
                IRBuilder<> b(call);
                Value *replacement = nullptr;
                switch (call->getIntrinsicID()) {
                case Intrinsic::fmuladd:
                    replacement =
                        b.CreateFAdd(b.CreateFMul(call->getArgOperand(0),
                                                  call->getArgOperand(1)),
                                     call->getArgOperand(2));
                    break;
                case Intrinsic::smax:
                    replacement = b.CreateSelect(
                        b.CreateICmpSGT(call->getArgOperand(0),
                                        call->getArgOperand(1)),
                        call->getArgOperand(0), call->getArgOperand(1));
                    break;
                case Intrinsic::umin:
                    replacement = b.CreateSelect(
                        b.CreateICmpULT(call->getArgOperand(0),
                                        call->getArgOperand(1)),
                        call->getArgOperand(0), call->getArgOperand(1));
                    break;
                default:
                    continue;
                }
                call->replaceAllUsesWith(replacement);
                call->eraseFromParent();
            }
        }
    }
}
#endif

int main(int argc, char **argv) {
    using namespace llvm;
    cl::opt<unsigned> frames("frames",
                             cl::desc("Number of simulation frames (>0)"),
                             cl::init(20000));
#ifdef TASK3_EXECUTE
    constexpr const char *defaultOutput = "";
#else
    constexpr const char *defaultOutput = "-";
#endif
    cl::opt<std::string> output("o",
                                cl::desc("Output IR file ('-' for stdout)"),
                                cl::init(defaultOutput));
    cl::ParseCommandLineOptions(argc, argv, "task_3: LLVM API app generator\n");
    if (frames == 0) {
        errs() << "--frames must be greater than zero\n";
        return 1;
    }
    LLVMContext context;
    auto module = buildAppModule(context, frames);
    if (verifyModule(*module, &errs()))
        return 1;
    if (!output.empty()) {
        std::error_code error;
        raw_fd_ostream stream(output, error, sys::fs::OF_Text);
        if (error) {
            errs() << "Cannot open " << output << ": " << error.message()
                   << '\n';
            return 1;
        }
        module->print(stream, nullptr);
        stream.flush();
        if (stream.has_error()) {
            errs() << "Cannot write " << output << '\n';
            return 1;
        }
    }
#ifdef TASK3_EXECUTE
    prepareForInterpreter(*module);
    if (verifyModule(*module, &errs()))
        return 1;
    auto *app = module->getFunction("app");
    std::string error;
    std::unique_ptr<ExecutionEngine> engine(
        EngineBuilder(std::move(module))
            .setEngineKind(EngineKind::Interpreter)
            .setErrorStr(&error)
            .create());
    if (!engine) {
        errs() << "Cannot create LLVM Interpreter: " << error << '\n';
        return 1;
    }
    sys::DynamicLibrary::AddSymbol("lle_X_simPutPixel",
                                   reinterpret_cast<void *>(&putPixelAdapter));
    sys::DynamicLibrary::AddSymbol("lle_X_simFlush",
                                   reinterpret_cast<void *>(&flushAdapter));
    simInit();
    engine->runFunction(app, {});
    simExit();
#endif
    return 0;
}
