#include "runtime.h"
#include "../task_3/sim.h"
#include "ensure.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace task4 {
uint64_t floatBits(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}
float bitsFloat(uint64_t value) {
    uint32_t bits = static_cast<uint32_t>(value);
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}
int64_t signedBits(uint64_t value) {
    int64_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
namespace {
int64_t toInteger(float f) {
    ENSURE_MSG(std::isfinite(f) && f >= -0x1p63f && f < 0x1p63f,
               "float-to-integer conversion out of range");
    return static_cast<int64_t>(f);
}
int pixelInt(uint64_t value) {
    uint32_t low = static_cast<uint32_t>(value);
    int32_t result;
    std::memcpy(&result, &low, sizeof result);
    return result;
}
template <Opcode op>
uint64_t execute(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    // Template instantiation gives each opcode its own emulating function.
    // The switch is folded at compile time, not dispatched at execution time.
    const float x = bitsFloat(a), y = bitsFloat(b), z = bitsFloat(c);
    switch (op) {
    case Opcode::MovI:
    case Opcode::MovF:
        return a;
    case Opcode::AddI:
        return a + b;
    case Opcode::SubI:
        return a - b;
    case Opcode::MulI:
        return a * b;
    case Opcode::DivI:
        ENSURE_MSG(b != 0, "integer division by zero");
        return a / b;
    case Opcode::AndI:
        return a & b;
    case Opcode::XorI:
        return a ^ b;
    case Opcode::OrI:
        return a | b;
    case Opcode::AddF:
        return floatBits(x + y);
    case Opcode::SubF:
        return floatBits(x - y);
    case Opcode::MulF:
        return floatBits(x * y);
    case Opcode::DivF:
        return floatBits(x / y);
    case Opcode::EqI:
        return a == b;
    case Opcode::LtI:
        return signedBits(a) < signedBits(b);
    case Opcode::LtF:
        return x < y;
    case Opcode::GtF:
        return x > y;
    case Opcode::SelI:
    case Opcode::SelF:
        return a ? b : c;
    case Opcode::IToF:
        return floatBits(static_cast<float>(signedBits(a)));
    case Opcode::FToI:
        return static_cast<uint64_t>(toInteger(x));
    case Opcode::Load:
        return floatBits(
            reinterpret_cast<float *>(static_cast<uintptr_t>(a))[b]);
    case Opcode::Store:
        reinterpret_cast<float *>(static_cast<uintptr_t>(a))[b] = z;
        return 0;
    case Opcode::Fill:
        std::fill_n(reinterpret_cast<float *>(static_cast<uintptr_t>(a)) + b, c,
                    bitsFloat(d));
        return 0;
    case Opcode::Madd: {
        const float product = x * y;
        return floatBits(product + z);
    }
    case Opcode::Shade: {
        const float difference = x - y;
        const float product = difference * z;
        return static_cast<uint64_t>(
            std::clamp<int64_t>(toInteger(product), 0, 255));
    }
    case Opcode::Pixel:
        simPutPixel(pixelInt(a), pixelInt(b), pixelInt(c));
        return 0;
    case Opcode::Flush:
        simFlush();
        return 0;
    default:
        ENSURE_MSG(false, "control instruction is not a runtime call");
    }
}
constexpr Emulator functions[] = {
#define OP(id, name, sig) &execute<Opcode::id>,
#include "instructions.def"
#undef OP
};
constexpr const char *names[] = {
#define OP(id, name, sig) "emu_" #id,
#include "instructions.def"
#undef OP
};
} // namespace
Emulator emulator(Opcode op) { return functions[static_cast<size_t>(op)]; }
const char *emulatorName(Opcode op) { return names[static_cast<size_t>(op)]; }
} // namespace task4
