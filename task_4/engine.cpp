#include "engine.h"
#include "../task_3/sim.h"
#include "ensure.h"
#include "runtime.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"

namespace task4 {
void execute(std::unique_ptr<llvm::Module> module) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    std::string error;
    std::unique_ptr<llvm::ExecutionEngine> engine(
        llvm::EngineBuilder(std::move(module))
            .setEngineKind(llvm::EngineKind::JIT)
            .setOptLevel(llvm::CodeGenOptLevel::None)
            .setErrorStr(&error)
            .create());
    ENSURE_MSG(engine, "cannot create Execution Engine: " + error);
    engine->addGlobalMapping("simPutPixel",
                             reinterpret_cast<uintptr_t>(&simPutPixel));
    engine->addGlobalMapping("simFlush",
                             reinterpret_cast<uintptr_t>(&simFlush));
    for (size_t i = 0; i < static_cast<size_t>(Opcode::Count); ++i) {
        auto op = static_cast<Opcode>(i);
        if (!control(op))
            engine->addGlobalMapping(emulatorName(op),
                                     reinterpret_cast<uintptr_t>(emulator(op)));
    }
    engine->finalizeObject();
    ENSURE_MSG(!engine->hasError(), engine->getErrorMessage());
    auto address = engine->getFunctionAddress("app");
    ENSURE_MSG(address, "Execution Engine did not resolve app");
    simInit();
    reinterpret_cast<void (*)()>(static_cast<uintptr_t>(address))();
    simExit();
}
} // namespace task4
