#pragma once
#include "asm.h"
#include <array>

namespace task4 {
// Uniform runtime ABI: integers unchanged, float32 in the low 32 bits,
// pointers as uintptr_t. Memory operands expand to (base, element index).
using Emulator = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t);
uint64_t floatBits(float value);
float bitsFloat(uint64_t value);
int64_t signedBits(uint64_t value);
Emulator emulator(Opcode op);
const char *emulatorName(Opcode op);
} // namespace task4
