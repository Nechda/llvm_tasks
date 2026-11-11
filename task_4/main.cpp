#include "asm.h"
#include "engine.h"
#include "ensure.h"
#include "ir.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>

namespace {
void optimize(llvm::Module &module) {
    llvm::LoopAnalysisManager loops;
    llvm::FunctionAnalysisManager functions;
    llvm::CGSCCAnalysisManager cgscc;
    llvm::ModuleAnalysisManager modules;
    llvm::PassBuilder builder;
    builder.registerLoopAnalyses(loops);
    builder.registerFunctionAnalyses(functions);
    builder.registerCGSCCAnalyses(cgscc);
    builder.registerModuleAnalyses(modules);
    builder.crossRegisterProxies(loops, functions, cgscc, modules);

    auto pipeline =
        builder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
    pipeline.run(module, modules);
}
} // namespace

int main(int argc, char **argv) {
    try {
        const std::string usage =
            "usage: asm2ir input.asm [output.ll] [--cfg|--direct|--calls] "
            "[--opt] [--run] [--frames N]";
        std::string source, destination, mode = "--direct";
        unsigned frames = 0;
        bool run = false;
        bool opt = false;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--run") {
                run = true;
            } else if (arg == "--opt") {
                opt = true;
            } else if (arg == "--cfg" || arg == "--direct" ||
                       arg == "--calls") {
                mode = arg;
            } else if (arg == "--frames" && i + 1 < argc) {
                std::string value = argv[++i];
                size_t end;
                auto count = std::stoull(value, &end);
                ENSURE_MSG(end == value.size() && count != 0 &&
                               count <= 0x7fffffff,
                           "--frames must be in [1, 2147483647]");
                frames = static_cast<unsigned>(count);
            } else if (arg.empty() || arg[0] == '-') {
                ENSURE_MSG(false, usage);
            } else if (source.empty()) {
                source = arg;
            } else if (destination.empty()) {
                destination = arg;
            } else {
                ENSURE_MSG(false, usage);
            }
        }
        ENSURE_MSG(!source.empty() && (!destination.empty() || run) &&
                       (mode != "--cfg" || !run), usage);
        std::ifstream input(source);
        ENSURE_MSG(input, "cannot open input assembly");
        auto program = task4::parse(input, frames);
        llvm::LLVMContext context;
        auto module = mode == "--cfg" ? task4::generateCFG(program, context)
                                      : task4::generateIR(program, context,
                                                          mode == "--calls");
        ENSURE_MSG(!llvm::verifyModule(*module, &llvm::errs()),
                   "invalid generated module");
        if (opt) {
            optimize(*module);
            ENSURE_MSG(!llvm::verifyModule(*module, &llvm::errs()),
                       "invalid optimized module");
        }
        if (!destination.empty()) {
            std::error_code error;
            llvm::raw_fd_ostream output(destination, error,
                                        llvm::sys::fs::OF_Text);
            ENSURE_MSG(!error, error.message());
            module->print(output, nullptr);
            output.flush();
            ENSURE(!output.has_error());
        }
        std::cerr << "instructions=" << program.instructions.size()
                  << " blocks=" << program.blocks.size()
                  << " registers=" << program.registers.size() << '\n';
        if (run)
            task4::execute(std::move(module));
    } catch (const std::exception &e) {
        std::cerr << "asm2ir: " << e.what() << '\n';
        return 1;
    }
}
