#pragma once
#include "llvm/IR/Module.h"
#include <memory>

namespace task4 {
void execute(std::unique_ptr<llvm::Module> module);
}
