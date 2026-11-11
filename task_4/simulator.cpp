#include "simulator.h"
#include "../task_3/sim.h"
#include "ensure.h"
#include "runtime.h"

namespace task4 {
namespace {
class Simulator final : public Visitor {
    const Program &p;
    std::vector<uint64_t> registers;
    std::vector<bool> initialized;
    std::vector<std::vector<float>> buffers;
    size_t pc = 0;

    uint64_t reg(size_t id) const {
        ENSURE_MSG(initialized[id],
                   "uninitialized register: " + p.registers[id].name);
        return registers[id];
    }
    uint64_t read(const Operand &a) const {
        switch (a.kind) {
        case Operand::Reg:
            return reg(a.id);
        case Operand::Integer:
        case Operand::Condition:
            return a.integer;
        case Operand::Float:
            return floatBits(a.floating);
        case Operand::BufferRef:
        case Operand::Memory:
            return reinterpret_cast<uintptr_t>(buffers[a.id].data());
        case Operand::Label:
            return a.id;
        }
        ENSURE_MSG(false, "invalid operand");
    }
    void write(size_t id, uint64_t value) {
        initialized[id] = true;
        registers[id] = value;
    }

  public:
    explicit Simulator(const Program &program)
        : p(program), registers(p.registers.size()),
          initialized(p.registers.size())
    {
        for (const auto &buffer : p.buffers)
            buffers.emplace_back(buffer.count);
    }
    void visit(const Instruction &inst) override {
        const auto &a = inst.args;
        size_t next = pc + 1;
        switch (inst.op) {
        case Opcode::Halt:
            next = p.instructions.size();
            break;
        case Opcode::Jump:
            next = a[0].id;
            break;
        case Opcode::Branch:
            next = read(a[0]) ? a[1].id : a[2].id;
            break;
        case Opcode::Loop: {
            auto counter = read(a[0]), step = read(a[1]), limit = read(a[2]);
            auto updated = counter + step;
            write(a[0].id, updated);
            if (signedBits(updated) < signedBits(limit))
                next = a[3].id;
            break;
        }
        default: {
            std::array<uint64_t, 4> args{};
            size_t slot = 0;
            for (size_t i = hasResult(inst.op) ? 1 : 0; i < a.size(); ++i) {
                args.at(slot++) = read(a[i]);
                if (a[i].kind == Operand::Memory) {
                    auto index = a[i].indexIsRegister ? reg(a[i].indexRegister)
                                                      : a[i].indexConstant;
                    ENSURE_MSG(index < p.buffers[a[i].id].count,
                               "memory index out of bounds");
                    args.at(slot++) = index;
                }
            }
            if (inst.op == Opcode::Fill) {
                auto size = p.buffers[a[0].id].count;
                ENSURE_MSG(args[1] <= size && args[2] <= size - args[1],
                           "fill range out of bounds");
            }
            auto result = emulator(inst.op)(args[0], args[1], args[2], args[3]);
            if (hasResult(inst.op))
                write(a[0].id, result);
            break;
        }
        }
        pc = next;
    }
    void run() {
        while (pc < p.instructions.size()) {
            const auto &inst = p.instructions[pc];
            try {
                visit(inst);
            } catch (const std::exception &e) {
                ENSURE_MSG(false, "line " + std::to_string(inst.line) + ": " +
                                      e.what());
            }
        }
    }
};
} // namespace
void simulate(const Program &program) {
    Simulator simulator(program);
    simInit();
    simulator.run();
    simExit();
}
} // namespace task4
