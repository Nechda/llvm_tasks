#include "ir.h"
#include "ensure.h"
#include "runtime.h"
#include "llvm/IR/IRBuilder.h"

namespace task4 {
std::unique_ptr<llvm::Module> generateCFG(const Program &p,
                                          llvm::LLVMContext &ctx) {
    auto module = std::make_unique<llvm::Module>("assembly.cfg", ctx);
    llvm::IRBuilder<> b(ctx);
    auto *fn =
        llvm::Function::Create(llvm::FunctionType::get(b.getVoidTy(), false),
                               llvm::Function::ExternalLinkage, "app", *module);
    std::vector<llvm::BasicBlock *> blocks;
    for (const auto &block : p.blocks)
        blocks.push_back(llvm::BasicBlock::Create(ctx, block.name, fn));
    for (size_t i = 0; i < p.blocks.size(); ++i) {
        b.SetInsertPoint(blocks[i]);
        const auto &successors = p.blocks[i].successors;
        if (successors.empty())
            b.CreateRetVoid();
        else if (successors.size() == 1)
            b.CreateBr(blocks[successors[0]]);
        else
            b.CreateCondBr(llvm::PoisonValue::get(b.getInt1Ty()),
                           blocks[successors[0]], blocks[successors[1]]);
    }
    return module;
}
namespace {
using namespace llvm;
class Generator final : public Visitor {
    const Program &p;
    std::unique_ptr<Module> module;
    IRBuilder<> b;
    Function *fn;
    std::vector<BasicBlock *> blocks;
    std::vector<AllocaInst *> registers, buffers;
    size_t pc = 0;
    bool calls;

    llvm::Type *type(task4::Type t) {
        switch (t) {
        case task4::Type::Integer:
            return b.getInt64Ty();
        case task4::Type::Float:
            return b.getFloatTy();
        case task4::Type::Condition:
            return b.getInt1Ty();
        }
        ENSURE_MSG(false, "invalid register type");
    }
    Value *read(const Operand &a) {
        switch (a.kind) {
        case Operand::Reg:
            return b.CreateLoad(type(p.registers[a.id].type), registers[a.id]);
        case Operand::Integer:
            return b.getInt64(a.integer);
        case Operand::Condition:
            return b.getInt1(a.integer);
        case Operand::Float:
            return ConstantFP::get(b.getFloatTy(), a.floating);
        case Operand::BufferRef:
            return buffers[a.id];
        case Operand::Memory: {
            if (calls)
                return buffers[a.id];
            auto *index =
                a.indexIsRegister
                    ? b.CreateLoad(b.getInt64Ty(), registers[a.indexRegister])
                    : static_cast<Value *>(b.getInt64(a.indexConstant));
            return b.CreateGEP(b.getFloatTy(), buffers[a.id], index);
        }
        case Operand::Label:
            return nullptr;
        }
        ENSURE_MSG(false, "invalid operand");
    }

    // Load every source before writing any destination. Instruction lowering
    // receives ready-to-use Values, including indexed memory addresses.
    struct Arguments {
        std::vector<Value *> values;
        Value *operator[](size_t i) const { return values.at(i); }
        Arguments(Generator &g, const Instruction &inst) {
            for (size_t i = 0; i < inst.args.size(); ++i)
                values.push_back(i == 0 && hasResult(inst.op) &&
                                         inst.op != Opcode::Loop
                                     ? nullptr
                                     : g.read(inst.args[i]));
        }
    };
    BasicBlock *target(const Operand &a) {
        return blocks[p.instructionBlock[a.id]];
    }

    Value *encode(Value *v) {
        if (v->getType()->isFloatTy())
            return b.CreateZExt(b.CreateBitCast(v, b.getInt32Ty()),
                                b.getInt64Ty());
        if (v->getType()->isPointerTy())
            return b.CreatePtrToInt(v, b.getInt64Ty());
        return b.CreateZExtOrTrunc(v, b.getInt64Ty());
    }
    Value *lowerCall(const Instruction &inst, const Arguments &args) {
        std::vector<Value *> packed;
        for (size_t i = hasResult(inst.op) ? 1 : 0; i < inst.args.size(); ++i) {
            packed.push_back(encode(args[i]));
            const auto &a = inst.args[i];
            if (a.kind == Operand::Memory)
                packed.push_back(
                    a.indexIsRegister
                        ? b.CreateLoad(b.getInt64Ty(),
                                       registers[a.indexRegister])
                        : static_cast<Value *>(b.getInt64(a.indexConstant)));
        }
        packed.resize(4, b.getInt64(0));
        auto callee = module->getOrInsertFunction(
            emulatorName(inst.op),
            FunctionType::get(b.getInt64Ty(), {b.getInt64Ty(), b.getInt64Ty(),
                                               b.getInt64Ty(), b.getInt64Ty()},
                              false));
        Value *result = b.CreateCall(callee, packed);
        if (!hasResult(inst.op))
            return nullptr;
        auto *dstType = type(p.registers[inst.args[0].id].type);
        if (dstType->isFloatTy())
            return b.CreateBitCast(b.CreateTrunc(result, b.getInt32Ty()),
                                   dstType);
        return b.CreateZExtOrTrunc(result, dstType);
    }

    void fill(Value *ptr, Value *start, Value *count, Value *value) {
        // A real IR loop, not a runtime emulation call. Zero count is valid.
        auto *before = b.GetInsertBlock();
        auto *test = BasicBlock::Create(b.getContext(), "fill.test", fn);
        auto *body = BasicBlock::Create(b.getContext(), "fill.body", fn);
        auto *done = BasicBlock::Create(b.getContext(), "fill.done", fn);
        b.CreateBr(test);
        b.SetInsertPoint(test);
        auto *i = b.CreatePHI(b.getInt64Ty(), 2);
        i->addIncoming(b.getInt64(0), before);
        b.CreateCondBr(b.CreateICmpULT(i, count), body, done);
        b.SetInsertPoint(body);
        b.CreateStore(value,
                      b.CreateGEP(b.getFloatTy(), ptr, b.CreateAdd(start, i)));
        auto *next = b.CreateAdd(i, b.getInt64(1));
        i->addIncoming(next, body);
        b.CreateBr(test);
        b.SetInsertPoint(done);
    }

    Value *lower(const Instruction &inst, const Arguments &a) {
        switch (inst.op) {
        case Opcode::MovI:
        case Opcode::MovF:
            return a[1];
        case Opcode::AddI:
            return b.CreateAdd(a[1], a[2]);
        case Opcode::SubI:
            return b.CreateSub(a[1], a[2]);
        case Opcode::MulI:
            return b.CreateMul(a[1], a[2]);
        case Opcode::DivI:
            return b.CreateUDiv(a[1], a[2]);
        case Opcode::AndI:
            return b.CreateAnd(a[1], a[2]);
        case Opcode::XorI:
            return b.CreateXor(a[1], a[2]);
        case Opcode::OrI:
            return b.CreateOr(a[1], a[2]);
        case Opcode::AddF:
            return b.CreateFAdd(a[1], a[2]);
        case Opcode::SubF:
            return b.CreateFSub(a[1], a[2]);
        case Opcode::MulF:
            return b.CreateFMul(a[1], a[2]);
        case Opcode::DivF:
            return b.CreateFDiv(a[1], a[2]);
        case Opcode::EqI:
            return b.CreateICmpEQ(a[1], a[2]);
        case Opcode::LtI:
            return b.CreateICmpSLT(a[1], a[2]);
        case Opcode::LtF:
            return b.CreateFCmpOLT(a[1], a[2]);
        case Opcode::GtF:
            return b.CreateFCmpOGT(a[1], a[2]);
        case Opcode::SelI:
        case Opcode::SelF:
            return b.CreateSelect(a[1], a[2], a[3]);
        case Opcode::IToF:
            return b.CreateSIToFP(a[1], b.getFloatTy());
        case Opcode::FToI:
            return b.CreateFPToSI(a[1], b.getInt64Ty());
        case Opcode::Load:
            return b.CreateLoad(b.getFloatTy(), a[1]);
        case Opcode::Store:
            b.CreateStore(a[1], a[0]);
            return nullptr;
        case Opcode::Fill:
            fill(a[0], a[1], a[2], a[3]);
            return nullptr;
        case Opcode::Madd:
            return b.CreateFAdd(b.CreateFMul(a[1], a[2]), a[3]);
        case Opcode::Shade: {
            auto *v = b.CreateFPToSI(
                b.CreateFMul(b.CreateFSub(a[1], a[2]), a[3]), b.getInt64Ty());
            v = b.CreateSelect(b.CreateICmpSLT(v, b.getInt64(0)), b.getInt64(0),
                               v);
            return b.CreateSelect(b.CreateICmpSGT(v, b.getInt64(255)),
                                  b.getInt64(255), v);
        }
        case Opcode::Pixel: {
            auto callee = module->getOrInsertFunction(
                "simPutPixel",
                FunctionType::get(
                    b.getVoidTy(),
                    {b.getInt32Ty(), b.getInt32Ty(), b.getInt32Ty()}, false));
            b.CreateCall(callee, {b.CreateTrunc(a[0], b.getInt32Ty()),
                                  b.CreateTrunc(a[1], b.getInt32Ty()),
                                  b.CreateTrunc(a[2], b.getInt32Ty())});
            return nullptr;
        }
        case Opcode::Flush:
            b.CreateCall(module->getOrInsertFunction(
                "simFlush", FunctionType::get(b.getVoidTy(), false)));
            return nullptr;
        case Opcode::Jump:
            b.CreateBr(target(inst.args[0]));
            return nullptr;
        case Opcode::Branch:
            b.CreateCondBr(
                b.CreateICmpNE(a[0], ConstantInt::get(a[0]->getType(), 0)),
                target(inst.args[1]), target(inst.args[2]));
            return nullptr;
        case Opcode::Loop: {
            auto *next = b.CreateAdd(a[0], a[1]);
            b.CreateStore(next, registers[inst.args[0].id]);
            b.CreateCondBr(b.CreateICmpSLT(next, a[2]), target(inst.args[3]),
                           blocks[p.instructionBlock[pc + 1]]);
            return nullptr;
        }
        case Opcode::Halt:
            b.CreateRetVoid();
            return nullptr;
        case Opcode::Count:
            break;
        }
        ENSURE_MSG(false, "missing instruction lowering");
    }

  public:
    Generator(const Program &program, LLVMContext &ctx, bool useCalls)
        : p(program), module(std::make_unique<Module>("assembly", ctx)), b(ctx),
          calls(useCalls)
    {
        fn = Function::Create(FunctionType::get(b.getVoidTy(), false),
                              Function::ExternalLinkage, "app", *module);
        // Pass 1: create the complete assembly CFG before emitting operations.
        auto *entry = BasicBlock::Create(ctx, "allocations", fn);
        for (const auto &block : p.blocks)
            blocks.push_back(BasicBlock::Create(ctx, block.name, fn));
        // Pass 2: registers discovered by the reader get one entry-block
        // alloca.
        b.SetInsertPoint(entry);
        for (const auto &reg : p.registers)
            registers.push_back(
                b.CreateAlloca(type(reg.type), nullptr, reg.name));
        for (const auto &buffer : p.buffers)
            buffers.push_back(b.CreateAlloca(
                b.getFloatTy(), b.getInt64(buffer.count), buffer.name));
        b.CreateBr(blocks[0]);
    }
    void visit(const Instruction &inst) override {
        const auto block = p.instructionBlock[pc];
        if (pc == p.blocks[block].begin) {
            if (b.GetInsertBlock()->empty() ||
                !b.GetInsertBlock()->back().isTerminator())
                b.CreateBr(blocks[block]);
            b.SetInsertPoint(blocks[block]);
        }
        Arguments args(*this, inst);
        if (auto *value = calls && !control(inst.op) ? lowerCall(inst, args)
                                                     : lower(inst, args))
            b.CreateStore(value, registers[inst.args[0].id]);
        ++pc;
    }
    std::unique_ptr<Module> run() {
        // Pass 3: the Visitor lowers instructions using already-loaded
        // operands.
        walk(p, *this);
        return std::move(module);
    }
};
} // namespace
std::unique_ptr<llvm::Module> generateIR(const Program &p,
                                         llvm::LLVMContext &ctx, bool calls) {
    return Generator(p, ctx, calls).run();
}
} // namespace task4
