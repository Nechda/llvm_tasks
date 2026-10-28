#include "app_ir_gen.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"

#include <array>
#include <cassert>

using namespace llvm;

std::unique_ptr<Module> buildAppModule(LLVMContext &context, uint32_t frames) {
    assert(frames > 0);
    auto module = std::make_unique<Module>("app", context);
    module->setSourceFileName("task_1/app.c");
    IRBuilder<NoFolder> b(context);
    auto *i16 = b.getInt16Ty();
    auto *i32 = b.getInt32Ty();
    auto *i64 = b.getInt64Ty();
    auto *f32 = b.getFloatTy();
    auto *v2f = FixedVectorType::get(f32, 2);
    auto *row = ArrayType::get(f32, 512);
    auto *plane = ArrayType::get(row, 256);
    auto *field = ArrayType::get(plane, 2);
    auto *triangle = ArrayType::get(i32, 3);
    auto f = [&](double x) { return ConstantFP::get(f32, x); };
    auto c32 = [&](int64_t x) { return b.getInt32(x); };
    auto c64 = [&](int64_t x) { return b.getInt64(x); };
    auto pixel = module->getOrInsertFunction(
        "simPutPixel",
        FunctionType::get(b.getVoidTy(), {i32, i32, i32}, false));
    // The input C uses an old-style declaration: simFlush(...).
    auto flush = module->getOrInsertFunction(
        "simFlush", FunctionType::get(b.getVoidTy(), true));
    auto *app = Function::Create(FunctionType::get(b.getVoidTy(), false),
                                 Function::ExternalLinkage, "app", *module);

    // Indices match the numbered SSA values and blocks in app_Os.ir.
    std::array<Value *, 233> v{};
    std::array<BasicBlock *, 233> bb{};
    for (unsigned n : {0,  10, 14, 17,  20,  25,  38,  45,  49,  50,  62,
                       64, 66, 89, 182, 187, 191, 194, 206, 209, 215, 218})
        bb[n] = BasicBlock::Create(context, "bb" + std::to_string(n), app);
    auto at = [&](unsigned n) { b.SetInsertPoint(bb[n]); };
    auto phi = [&](unsigned n, Type *type) {
        v[n] = b.CreatePHI(type, 2, "v" + std::to_string(n));
    };
    auto incoming = [&](unsigned n, Value *value, unsigned block) {
        cast<PHINode>(v[n])->addIncoming(value, bb[block]);
    };
    auto gep = [&](Type *type, Value *ptr, Value *index) {
        return b.CreateInBoundsGEP(type, ptr, index);
    };
    auto load = [&](Value *ptr) {
        return b.CreateAlignedLoad(f32, ptr, Align(4));
    };
    auto store = [&](Value *value, Value *ptr) {
        b.CreateAlignedStore(value, ptr, Align(4));
    };
    auto add = [&](Value *x, Value *y) {
        return b.CreateAdd(x, y, "", true, true);
    };
    auto fma = [&](Value *x, Value *y, Value *z) {
        return b.CreateIntrinsic(Intrinsic::fmuladd, {x->getType()}, {x, y, z});
    };

    at(0);
    for (unsigned n : {1, 2, 3}) {
        auto *a = b.CreateAlloca(field);
        a->setAlignment(Align(16));
        v[n] = a;
    }
    v[4] = b.CreateAlloca(triangle);
    v[5] = b.CreateAlloca(triangle);
    store(c32(256), v[4]);
    v[6] = gep(b.getInt8Ty(), v[4], c64(4));
    store(c32(205), v[6]);
    v[7] = gep(b.getInt8Ty(), v[4], c64(8));
    store(c32(307), v[7]);
    store(c32(68), v[5]);
    v[8] = gep(b.getInt8Ty(), v[5], c64(4));
    store(c32(158), v[8]);
    v[9] = gep(b.getInt8Ty(), v[5], c64(8));
    store(c32(158), v[9]);
    b.CreateMemSet(v[2], b.getInt8(0), 524288, Align(16));
    b.CreateMemSet(v[3], b.getInt8(0), 524288, Align(16));
    b.CreateBr(bb[10]);

    at(10);
    phi(11, i64);
    v[12] = gep(row, v[1], v[11]);
    v[13] = b.CreateTrunc(v[11], i32);
    b.CreateBr(bb[14]);
    at(14);
    phi(15, i64);
    v[16] = b.CreateTrunc(v[15], i32);
    b.CreateBr(bb[25]);
    at(17);
    v[18] = add(v[11], c64(1));
    v[19] = b.CreateICmpEQ(v[18], c64(256));
    b.CreateCondBr(v[19], bb[50], bb[10]);

    at(25);
    phi(26, i64);
    phi(27, f32);
    v[28] = gep(i32, v[4], v[26]);
    v[29] = b.CreateAlignedLoad(i32, v[28], Align(4));
    v[30] = b.CreateNSWSub(v[16], v[29]);
    v[31] = gep(i32, v[5], v[26]);
    v[32] = b.CreateAlignedLoad(i32, v[31], Align(4));
    v[33] = b.CreateNSWSub(v[13], v[32]);
    v[34] = b.CreateNSWMul(v[30], v[30]);
    v[35] = b.CreateNSWMul(v[33], v[33]);
    v[36] = add(v[35], v[34]);
    v[37] = b.CreateICmpULT(v[36], c32(400));
    b.CreateCondBr(v[37], bb[38], bb[45]);
    at(38);
    v[39] = b.CreateUIToFP(v[36], f32);
    v[40] = b.CreateFDiv(v[39], f(400));
    v[41] = b.CreateFSub(f(1), v[40]);
    v[42] = b.CreateFMul(v[41], f(0.5));
    v[43] = b.CreateFCmpOGT(v[42], v[27]);
    v[44] = b.CreateSelect(v[43], v[42], v[27]);
    b.CreateBr(bb[45]);
    at(45);
    phi(46, f32);
    v[47] = add(v[26], c64(1));
    v[48] = b.CreateICmpEQ(v[47], c64(3));
    b.CreateCondBr(v[48], bb[20], bb[25]);
    at(20);
    v[21] = b.CreateFAdd(v[46], f(1));
    v[22] = gep(f32, v[12], v[15]);
    store(v[21], v[22]);
    v[23] = add(v[15], c64(1));
    v[24] = b.CreateICmpEQ(v[23], c64(512));
    b.CreateCondBr(v[24], bb[17], bb[14]);
    at(49);
    b.CreateRetVoid();

    at(50);
    phi(51, i32);
    v[52] = b.CreateAnd(v[51], c32(1));
    v[53] = b.CreateXor(v[52], c32(1));
    v[54] = b.CreateZExt(v[52], i64);
    for (unsigned n = 55; n <= 57; ++n)
        v[n] = gep(plane, v[n - 54], v[54]);
    v[58] = b.CreateZExt(v[53], i64);
    for (unsigned n = 59; n <= 61; ++n)
        v[n] = gep(plane, v[n - 58], v[58]);
    b.CreateBr(bb[66]);
    at(66);
    phi(67, i64);
    v[68] = b.CreateICmpEQ(v[67], c64(0));
    v[69] = b.CreateICmpEQ(v[67], c64(255));
    v[70] = add(v[67], c64(1));
    v[71] = gep(row, v[55], v[67]);
    v[72] = b.CreateShl(v[67], c64(32));
    v[73] = b.CreateAdd(v[72], c64(-4294967296LL));
    v[74] = b.CreateAShr(v[73], c64(32), "", true);
    v[75] = b.CreateSelect(v[68], c64(255), v[74]);
    v[76] = gep(row, v[55], v[75]);
    v[77] = b.CreateAnd(v[70], c64(4294967295LL));
    v[78] = b.CreateSelect(v[69], c64(0), v[77]);
    v[79] = gep(row, v[55], v[78]);
    v[80] = gep(row, v[56], v[67]);
    v[81] = gep(row, v[57], v[75]);
    v[82] = gep(row, v[56], v[75]);
    v[83] = gep(row, v[57], v[78]);
    v[84] = gep(row, v[56], v[78]);
    v[85] = gep(row, v[57], v[67]);
    v[86] = gep(row, v[59], v[67]);
    v[87] = gep(row, v[60], v[67]);
    v[88] = gep(row, v[61], v[67]);
    b.CreateBr(bb[89]);

    at(89);
    phi(90, i64);
    v[91] = b.CreateICmpEQ(v[90], c64(0));
    v[92] = b.CreateICmpEQ(v[90], c64(511));
    v[93] = add(v[90], c64(1));
    v[94] = b.CreateShl(v[90], c64(32));
    v[95] = b.CreateAdd(v[94], c64(-4294967296LL));
    v[96] = b.CreateAShr(v[95], c64(32), "", true);
    v[97] = b.CreateSelect(v[91], c64(511), v[96]);
    v[98] = gep(f32, v[71], v[97]);
    v[99] = load(v[98]);
    v[100] = b.CreateAnd(v[93], c64(4294967295LL));
    v[101] = b.CreateSelect(v[92], c64(0), v[100]);
    v[102] = gep(f32, v[71], v[101]);
    v[103] = load(v[102]);
    v[104] = gep(f32, v[76], v[90]);
    v[105] = load(v[104]);
    v[106] = gep(f32, v[79], v[90]);
    v[107] = load(v[106]);
    v[108] = gep(f32, v[80], v[97]);
    v[109] = load(v[108]);
    v[110] = gep(f32, v[80], v[101]);
    v[111] = load(v[110]);
    v[112] = b.CreateInsertElement(PoisonValue::get(v2f), v[111], c64(0));
    v[113] = b.CreateInsertElement(v[112], v[109], c64(1));
    v[114] = b.CreateFMul(v[113], v[113]);
    v[115] = b.CreateInsertElement(PoisonValue::get(v2f), v[103], c64(0));
    v[116] = b.CreateInsertElement(v[115], v[99], c64(1));
    v[117] = b.CreateFDiv(v[114], v[116]);
    auto *half = ConstantVector::getSplat(ElementCount::getFixed(2), f(0.5));
    v[118] = b.CreateFMul(v[116], half);
    v[119] = fma(v[118], v[116], v[117]);
    v[120] = gep(f32, v[81], v[90]);
    v[121] = load(v[120]);
    v[122] = gep(f32, v[82], v[90]);
    v[123] = load(v[122]);
    v[124] = gep(f32, v[83], v[90]);
    v[125] = load(v[124]);
    v[126] = gep(f32, v[84], v[90]);
    v[127] = load(v[126]);
    v[128] = b.CreateInsertElement(PoisonValue::get(v2f), v[125], c64(0));
    v[129] = b.CreateInsertElement(v[128], v[121], c64(1));
    v[130] = b.CreateInsertElement(PoisonValue::get(v2f), v[127], c64(0));
    v[131] = b.CreateInsertElement(v[130], v[123], c64(1));
    v[132] = b.CreateFMul(v[129], v[131]);
    v[133] = b.CreateInsertElement(PoisonValue::get(v2f), v[107], c64(0));
    v[134] = b.CreateInsertElement(v[133], v[105], c64(1));
    v[135] = b.CreateFDiv(v[132], v[134]);
    v[136] = gep(f32, v[85], v[97]);
    v[137] = load(v[136]);
    v[138] = gep(f32, v[85], v[101]);
    v[139] = load(v[138]);
    v[140] = b.CreateInsertElement(PoisonValue::get(v2f), v[139], c64(0));
    v[141] = b.CreateInsertElement(v[140], v[137], c64(1));
    v[142] = b.CreateFMul(v[113], v[141]);
    v[143] = b.CreateFDiv(v[142], v[116]);
    v[144] = b.CreateFMul(v[129], v[129]);
    v[145] = b.CreateFDiv(v[144], v[134]);
    v[146] = b.CreateFMul(v[134], half);
    v[147] = fma(v[146], v[134], v[145]);
    v[148] = b.CreateFAdd(v[99], v[103]);
    v[149] = b.CreateFAdd(v[148], v[105]);
    v[150] = b.CreateFAdd(v[149], v[107]);
    v[151] = b.CreateFMul(v[150], f(0.25));
    v[152] = b.CreateFAdd(v[109], v[111]);
    v[153] = b.CreateFAdd(v[152], v[123]);
    v[154] = b.CreateFAdd(v[153], v[127]);
    v[155] = b.CreateFMul(v[154], f(0.25));
    v[156] = b.CreateFAdd(v[137], v[139]);
    v[157] = b.CreateFAdd(v[121], v[156]);
    v[158] = b.CreateFAdd(v[125], v[157]);
    v[159] = b.CreateFMul(v[158], f(0.25));
    v[160] = b.CreateFSub(v[111], v[109]);
    v[161] = b.CreateFSub(v[125], v[121]);
    v[162] = b.CreateFAdd(v[160], v[161]);
    v[163] = fma(v[162], f(-0.1f), v[151]);
    v[164] = gep(f32, v[86], v[90]);
    store(v[163], v[164]);
    auto shuffle = [&](Value *x) {
        return b.CreateShuffleVector(x, PoisonValue::get(v2f), {1, -1});
    };
    v[165] = shuffle(v[119]);
    v[166] = b.CreateFSub(v[119], v[165]);
    v[167] = shuffle(v[135]);
    v[168] = b.CreateFSub(v[135], v[167]);
    v[169] = b.CreateFAdd(v[166], v[168]);
    v[170] = b.CreateExtractElement(v[169], c64(0));
    v[171] = fma(v[170], f(-0.1f), v[155]);
    v[172] = gep(f32, v[87], v[90]);
    store(v[171], v[172]);
    v[173] = shuffle(v[143]);
    v[174] = b.CreateFSub(v[143], v[173]);
    v[175] = shuffle(v[147]);
    v[176] = b.CreateFSub(v[147], v[175]);
    v[177] = b.CreateFAdd(v[176], v[174]);
    v[178] = b.CreateExtractElement(v[177], c64(0));
    v[179] = fma(v[178], f(-0.1f), v[159]);
    v[180] = gep(f32, v[88], v[90]);
    store(v[179], v[180]);
    v[181] = b.CreateICmpEQ(v[93], c64(512));
    b.CreateCondBr(v[181], bb[62], bb[89]);
    at(62);
    v[63] = b.CreateICmpEQ(v[70], c64(256));
    b.CreateCondBr(v[63], bb[64], bb[66]);
    at(64);
    v[65] = b.CreateAlignedLoad(f32, v[59], Align(16));
    b.CreateBr(bb[182]);

    at(182);
    phi(183, i64);
    phi(184, f32);
    phi(185, f32);
    v[186] = gep(row, v[59], v[183]);
    b.CreateBr(bb[194]);
    at(194);
    phi(195, i64);
    phi(196, f32);
    phi(197, f32);
    v[198] = gep(f32, v[186], v[195]);
    v[199] = load(v[198]);
    v[200] = b.CreateFCmpOLT(v[199], v[197]);
    v[201] = b.CreateSelect(v[200], v[199], v[197]);
    v[202] = b.CreateFCmpOGT(v[199], v[196]);
    v[203] = b.CreateSelect(v[202], v[199], v[196]);
    v[204] = add(v[195], c64(1));
    v[205] = b.CreateICmpEQ(v[204], c64(512));
    b.CreateCondBr(v[205], bb[191], bb[194]);
    at(191);
    v[192] = add(v[183], c64(1));
    v[193] = b.CreateICmpEQ(v[192], c64(256));
    b.CreateCondBr(v[193], bb[187], bb[182]);
    at(187);
    v[188] = b.CreateFSub(v[203], v[201]);
    v[189] = b.CreateFAdd(v[188], f(1e-9f));
    v[190] = b.CreateFDiv(f(255), v[189]);
    b.CreateBr(bb[209]);

    at(209);
    phi(210, i32);
    v[211] = b.CreateTrunc(v[210], i16);
    v[212] = b.CreateUDiv(v[211], b.getInt16(3));
    v[213] = b.CreateZExt(v[212], i64);
    v[214] = gep(row, v[59], v[213]);
    b.CreateBr(bb[218]);
    at(218);
    phi(219, i32);
    v[220] = b.CreateTrunc(v[219], i16);
    v[221] = b.CreateUDiv(v[220], b.getInt16(3));
    v[222] = b.CreateZExt(v[221], i64);
    v[223] = gep(f32, v[214], v[222]);
    v[224] = load(v[223]);
    v[225] = b.CreateFSub(v[224], v[201]);
    v[226] = b.CreateFMul(v[190], v[225]);
    v[227] = b.CreateFPToSI(v[226], i32);
    v[228] = b.CreateIntrinsic(Intrinsic::smax, {i32}, {v[227], c32(0)});
    v[229] = b.CreateIntrinsic(Intrinsic::umin, {i32}, {v[228], c32(255)});
    v[230] = b.CreateOr(v[229], c32(-16777216));
    b.CreateCall(pixel, {v[219], v[210], v[230]});
    v[231] = add(v[219], c32(1));
    v[232] = b.CreateICmpEQ(v[231], c32(1536));
    b.CreateCondBr(v[232], bb[215], bb[218]);
    at(215);
    v[216] = add(v[210], c32(1));
    v[217] = b.CreateICmpEQ(v[216], c32(768));
    b.CreateCondBr(v[217], bb[206], bb[209]);
    at(206);
    b.CreateCall(flush);
    v[207] = add(v[51], c32(1));
    v[208] = b.CreateICmpEQ(v[207], c32(frames));
    b.CreateCondBr(v[208], bb[49], bb[50]);

    incoming(11, c64(0), 0);
    incoming(11, v[18], 17);
    incoming(15, c64(0), 10);
    incoming(15, v[23], 20);
    incoming(26, c64(0), 14);
    incoming(26, v[47], 45);
    incoming(27, f(0), 14);
    incoming(27, v[46], 45);
    incoming(46, v[44], 38);
    incoming(46, v[27], 25);
    incoming(51, v[207], 206);
    incoming(51, c32(0), 17);
    incoming(67, c64(0), 50);
    incoming(67, v[70], 62);
    incoming(90, c64(0), 66);
    incoming(90, v[93], 89);
    incoming(183, c64(0), 64);
    incoming(183, v[192], 191);
    incoming(184, v[65], 64);
    incoming(184, v[203], 191);
    incoming(185, v[65], 64);
    incoming(185, v[201], 191);
    incoming(195, c64(0), 182);
    incoming(195, v[204], 194);
    incoming(196, v[184], 182);
    incoming(196, v[203], 194);
    incoming(197, v[185], 182);
    incoming(197, v[201], 194);
    incoming(210, c32(0), 187);
    incoming(210, v[216], 215);
    incoming(219, c32(0), 209);
    incoming(219, v[231], 218);
    // Keep readable SSA names even though blocks were built out of source
    // order.
    for (unsigned n = 1; n < v.size(); ++n)
        if (v[n])
            v[n]->setName("v" + std::to_string(n));
    return module;
}
