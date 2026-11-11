#include "asm.h"
#include "ensure.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <set>

namespace task4 {
namespace {
constexpr Spec specs[] = {
#define OP(id, name, signature) {name, signature},
#include "instructions.def"
#undef OP
};
std::string trim(std::string s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos)
        return {};
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
bool identifier(const std::string &s) {
    return !s.empty() &&
           (std::isalpha(static_cast<unsigned char>(s[0])) || s[0] == '_') &&
           std::all_of(s.begin(), s.end(), [](unsigned char c) {
               return std::isalnum(c) || c == '_';
           });
}
std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> result;
    if (s.empty())
        return result;
    size_t start = 0;
    do {
        auto end = s.find(',', start);
        result.push_back(trim(s.substr(start, end - start)));
        if (end == std::string::npos)
            break;
        start = end + 1;
    } while (true);
    return result;
}
uint64_t integer(const std::string &s) {
    ENSURE_MSG(!s.empty(), "empty integer literal");
    size_t end;
    // Base ten unless an explicit hexadecimal prefix is present.
    auto pos = s[0] == '-' || s[0] == '+' ? 1u : 0u;
    int base = s.compare(pos, 2, "0x") == 0 ? 16 : 10;
    uint64_t value = s[0] == '-'
                         ? static_cast<uint64_t>(std::stoll(s, &end, base))
                         : std::stoull(s, &end, base);
    ENSURE_MSG(end == s.size(), "invalid integer: " + s);
    return value;
}
class Reader {
    Program p;
    std::map<std::string, std::string> constants;
    std::map<std::string, size_t> regs, buffers;

    std::string resolve(const std::string &s) {
        auto it = constants.find(s);
        return it == constants.end() ? s : it->second;
    }
    Operand operand(std::string s, char expected) {
        ENSURE_MSG(!s.empty(), "empty operand");
        if (expected == 'l') {
            ENSURE_MSG(identifier(s), "invalid label: " + s);
            Operand a{Operand::Label};
            a.label = s;
            return a;
        }
        if (expected == 'b' || expected == 'm') {
            std::string index = "0";
            if (expected == 'm') {
                ENSURE_MSG(s.front() == '[' && s.back() == ']',
                           "expected [buffer + index]");
                s = trim(s.substr(1, s.size() - 2));
                auto plus = s.find('+');
                if (plus != std::string::npos) {
                    index = trim(s.substr(plus + 1));
                    s = trim(s.substr(0, plus));
                }
            }
            auto it = buffers.find(s);
            ENSURE_MSG(it != buffers.end(), "unknown buffer: " + s);
            Operand a{expected == 'm' ? Operand::Memory : Operand::BufferRef};
            a.id = it->second;
            if (expected == 'm') {
                auto i = operand(index, 'i');
                a.indexIsRegister = i.kind == Operand::Reg;
                a.indexRegister = i.id;
                a.indexConstant = i.integer;
            }
            return a;
        }
        const char lower = static_cast<char>(std::tolower(expected));
        Type type = lower == 'f'   ? Type::Float
                    : lower == 'c' ? Type::Condition
                                   : Type::Integer;
        bool reg = s.size() > 2 && s[1] == '_' &&
                   (s[0] == 'r' || s[0] == 'f' || s[0] == 'c');
        if (reg) {
            ENSURE_MSG(identifier(s) && s[0] == (lower == 'i' ? 'r' : lower),
                       "wrong register type: " + s);
            auto pair = regs.emplace(s, p.registers.size());
            if (pair.second)
                p.registers.push_back({s, type});
            Operand a{Operand::Reg};
            a.id = pair.first->second;
            return a;
        }
        ENSURE_MSG(!std::isupper(expected), "destination must be a register");
        s = resolve(s);
        Operand a{type == Type::Float       ? Operand::Float
                  : type == Type::Condition ? Operand::Condition
                                            : Operand::Integer};
        if (type == Type::Float) {
            size_t end;
            a.floating = std::stof(s, &end);
            ENSURE_MSG(end == s.size() && std::isfinite(a.floating),
                       "invalid float: " + s);
        } else {
            a.integer = integer(s);
            ENSURE_MSG(type != Type::Condition || a.integer <= 1,
                       "condition must be 0 or 1");
        }
        return a;
    }
    void cfg() {
        const size_t n = p.instructions.size();
        ENSURE_MSG(n != 0, "empty program");
        std::set<size_t> leaders{0};
        for (const auto &[name, index] : p.labels) {
            ENSURE_MSG(index != n, "label without instruction: " + name);
            leaders.insert(index);
        }
        for (size_t i = 0; i < n; ++i) {
            for (auto &arg : p.instructions[i].args) {
                if (arg.kind != Operand::Label)
                    continue;
                auto it = p.labels.find(arg.label);
                ENSURE_MSG(it != p.labels.end(),
                           "undefined label: " + arg.label);
                arg.id = it->second;
            }
            if (control(p.instructions[i].op) && i + 1 < n)
                leaders.insert(i + 1);
        }
        p.instructionBlock.resize(n);
        for (auto begin : leaders) {
            auto next = leaders.upper_bound(begin);
            size_t end = next == leaders.end() ? n : *next;
            std::string name = "block_" + std::to_string(begin);
            for (const auto &[label, index] : p.labels)
                if (index == begin) {
                    name = label;
                    break;
                }
            std::fill(p.instructionBlock.begin() + begin,
                      p.instructionBlock.begin() + end, p.blocks.size());
            p.blocks.push_back({begin, end, name, {}});
        }
        for (auto &block : p.blocks) {
            auto &tail = p.instructions[block.end - 1];
            for (const auto &a : tail.args)
                if (a.kind == Operand::Label)
                    block.successors.push_back(p.instructionBlock[a.id]);
            if (!control(tail.op) || tail.op == Opcode::Loop) {
                ENSURE_MSG(block.end != n, "program falls off the end");
                block.successors.push_back(p.instructionBlock[block.end]);
            }
        }
    }

  public:
    Program read(std::istream &in, unsigned frames) {
        std::string text;
        size_t line = 0;
        while (std::getline(in, text)) {
            ++line;
            try {
                text = trim(text.substr(0, text.find(';')));
                if (text.empty())
                    continue;
                if (text.back() == ':') {
                    auto name = trim(text.substr(0, text.size() - 1));
                    ENSURE_MSG(identifier(name) &&
                                   p.labels.emplace(name, p.instructions.size())
                                       .second,
                               "invalid or duplicate label: " + name);
                    continue;
                }
                auto space = text.find_first_of(" \t");
                auto name = text.substr(0, space);
                auto args = split(
                    space == std::string::npos ? "" : trim(text.substr(space)));
                if (name == ".const") {
                    ENSURE_MSG(args.size() == 2 && identifier(args[0]) &&
                                   !args[1].empty() &&
                                   constants.emplace(args[0], args[1]).second,
                               "invalid or duplicate .const");
                    if (frames && args[0] == "FRAMES")
                        constants[args[0]] = std::to_string(frames);
                    continue;
                }
                if (name == ".buffer") {
                    ENSURE_MSG(
                        args.size() == 3 && identifier(args[0]) &&
                            args[1] == "f32" &&
                            buffers.emplace(args[0], p.buffers.size()).second,
                        "invalid or duplicate .buffer");
                    auto count = integer(resolve(args[2]));
                    ENSURE_MSG(count != 0 && count <= (1ULL << 30),
                               "invalid buffer size");
                    p.buffers.push_back({args[0], count});
                    continue;
                }
                size_t op = 0;
                while (op < static_cast<size_t>(Opcode::Count) &&
                       name != specs[op].name)
                    ++op;
                ENSURE_MSG(op != static_cast<size_t>(Opcode::Count),
                           "unknown instruction: " + name);
                const char *sig = specs[op].signature;
                ENSURE_MSG(args.size() == std::strlen(sig),
                           "wrong operand count: " + name);
                Instruction inst{static_cast<Opcode>(op), {}, line};
                for (size_t j = 0; j < args.size(); ++j)
                    inst.args.push_back(operand(args[j], sig[j]));
                p.instructions.push_back(std::move(inst));
            } catch (const std::exception &e) {
                ENSURE_MSG(false,
                           "line " + std::to_string(line) + ": " + e.what());
            }
        }
        cfg();
        return std::move(p);
    }
};
} // namespace
const Spec &spec(Opcode op) { return specs[static_cast<size_t>(op)]; }
bool control(Opcode op) {
    return op == Opcode::Jump || op == Opcode::Branch || op == Opcode::Loop ||
           op == Opcode::Halt;
}
bool hasResult(Opcode op) {
    return std::isupper(static_cast<unsigned char>(spec(op).signature[0]));
}
Program parse(std::istream &input, unsigned frames) {
    return Reader().read(input, frames);
}
void walk(const Program &program, Visitor &visitor) {
    for (const auto &instruction : program.instructions)
        visitor.visit(instruction);
}
} // namespace task4
