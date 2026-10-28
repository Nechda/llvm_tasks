#pragma once

#include <cstdint>
#include <memory>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

// Build app from task_1/app_Os.ir without parsing IR or compiling C at runtime.
std::unique_ptr<llvm::Module> buildAppModule(llvm::LLVMContext &context,
                                             uint32_t frames = 20000);
