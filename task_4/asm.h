#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <map>
#include <string>
#include <vector>

namespace task4 {
enum class Type { Integer, Float, Condition };
enum class Opcode {
#define OP(id, name, signature) id,
#include "instructions.def"
#undef OP
    Count
};
struct Spec {
    const char *name;
    const char *signature;
};
const Spec &spec(Opcode op);
bool control(Opcode op);
bool hasResult(Opcode op);

struct Register {
    std::string name;
    Type type;
};
struct Buffer {
    std::string name;
    uint64_t count;
};
struct Operand {
    enum Kind {
        Reg,
        Integer,
        Float,
        Condition,
        Memory,
        BufferRef,
        Label
    } kind;
    size_t id = 0; // register, buffer, or resolved instruction index
    uint64_t integer = 0;
    float floating = 0;
    // A memory operand owns one scalar index (register or integer).
    bool indexIsRegister = false;
    size_t indexRegister = 0;
    uint64_t indexConstant = 0;
    std::string label;
};
struct Instruction {
    Opcode op;
    std::vector<Operand> args;
    size_t line;
};
struct Block {
    size_t begin, end; // instruction interval [begin, end)
    std::string name;
    std::vector<size_t> successors; // block indices
};
struct Program {
    std::vector<Register> registers;
    std::vector<Buffer> buffers;
    std::vector<Instruction> instructions;
    std::map<std::string, size_t> labels;
    std::vector<Block> blocks;
    std::vector<size_t> instructionBlock;
};

Program parse(std::istream &input, unsigned frames = 0);

// Shared instruction-reader interface. Generators visit all instructions;
// a simulator can select the next instruction after each visit.
class Visitor {
  public:
    virtual ~Visitor() = default;
    virtual void visit(const Instruction &instruction) = 0;
};
void walk(const Program &program, Visitor &visitor);
} // namespace task4
