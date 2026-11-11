#pragma once
#include "asm.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>

namespace task4 {
// CFG-only modules intentionally contain no data operations.
std::unique_ptr<llvm::Module> generateCFG(const Program &, llvm::LLVMContext &);
std::unique_ptr<llvm::Module> generateIR(const Program &, llvm::LLVMContext &,
                                         bool calls = false);
} // namespace task4
