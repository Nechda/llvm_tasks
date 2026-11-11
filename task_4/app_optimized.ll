; ModuleID = 'assembly'
source_filename = "assembly"

define void @app() local_unnamed_addr {
allocations:
  %h162 = alloca [262144 x float], align 4
  %hu163 = alloca [262144 x float], align 4
  %hv164 = alloca [262144 x float], align 4
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(524288) %hu163, i8 0, i64 524288, i1 false)
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(524288) %hv164, i8 0, i64 524288, i1 false)
  br label %init_y

init_y:                                           ; preds = %allocations, %block_30
  %r_y.0 = phi i64 [ %39, %block_30 ], [ 0, %allocations ]
  %.idx = shl nuw nsw i64 %r_y.0, 11
  %0 = getelementptr i8, ptr %h162, i64 %.idx
  %1 = add nsw i64 %r_y.0, -68
  %2 = mul i64 %1, %1
  %3 = add nsw i64 %r_y.0, -158
  %4 = mul i64 %3, %3
  br label %init_x

init_x:                                           ; preds = %bump_next.2, %init_y
  %r_x.0 = phi i64 [ 0, %init_y ], [ %37, %bump_next.2 ]
  %5 = add nsw i64 %r_x.0, -256
  %6 = mul nsw i64 %5, %5
  %7 = add i64 %6, %2
  %8 = icmp slt i64 %7, 400
  br i1 %8, label %bump_inside, label %bump_next

bump_inside:                                      ; preds = %init_x
  %9 = sitofp i64 %7 to float
  %10 = fdiv nnan float %9, 4.000000e+02
  %11 = fsub nnan float 1.000000e+00, %10
  %12 = fmul nnan float %11, 5.000000e-01
  %13 = fcmp ogt float %12, 0.000000e+00
  %14 = select i1 %13, float %12, float 0.000000e+00
  br label %bump_next

bump_next:                                        ; preds = %bump_inside, %init_x
  %f_bump.1 = phi float [ %14, %bump_inside ], [ 0.000000e+00, %init_x ]
  %15 = add nsw i64 %r_x.0, -205
  %16 = mul nsw i64 %15, %15
  %17 = add i64 %16, %4
  %18 = icmp slt i64 %17, 400
  br i1 %18, label %bump_inside.1, label %bump_next.1

bump_inside.1:                                    ; preds = %bump_next
  %19 = sitofp i64 %17 to float
  %20 = fdiv nnan float %19, 4.000000e+02
  %21 = fsub nnan float 1.000000e+00, %20
  %22 = fmul nnan float %21, 5.000000e-01
  %23 = fcmp ogt float %22, %f_bump.1
  %24 = select i1 %23, float %22, float %f_bump.1
  br label %bump_next.1

bump_next.1:                                      ; preds = %bump_inside.1, %bump_next
  %f_bump.1.1 = phi float [ %24, %bump_inside.1 ], [ %f_bump.1, %bump_next ]
  %25 = add nsw i64 %r_x.0, -307
  %26 = mul nsw i64 %25, %25
  %27 = add i64 %26, %4
  %28 = icmp slt i64 %27, 400
  br i1 %28, label %bump_inside.2, label %bump_next.2

bump_inside.2:                                    ; preds = %bump_next.1
  %29 = sitofp i64 %27 to float
  %30 = fdiv nnan float %29, 4.000000e+02
  %31 = fsub nnan float 1.000000e+00, %30
  %32 = fmul nnan float %31, 5.000000e-01
  %33 = fcmp ogt float %32, %f_bump.1.1
  %34 = select i1 %33, float %32, float %f_bump.1.1
  br label %bump_next.2

bump_next.2:                                      ; preds = %bump_inside.2, %bump_next.1
  %f_bump.1.2 = phi float [ %34, %bump_inside.2 ], [ %f_bump.1.1, %bump_next.1 ]
  %35 = fadd float %f_bump.1.2, 1.000000e+00
  %36 = getelementptr [4 x i8], ptr %0, i64 %r_x.0
  store float %35, ptr %36, align 4
  %37 = add nuw nsw i64 %r_x.0, 1
  %38 = icmp samesign ult i64 %r_x.0, 511
  br i1 %38, label %init_x, label %block_30

block_30:                                         ; preds = %bump_next.2
  %39 = add nuw nsw i64 %r_y.0, 1
  %40 = icmp samesign ult i64 %r_y.0, 255
  br i1 %40, label %init_y, label %frame

frame:                                            ; preds = %block_30, %block_150
  %r_step.0 = phi i64 [ %178, %block_150 ], [ 0, %block_30 ]
  %41 = shl nuw nsw i64 %r_step.0, 17
  %42 = and i64 %41, 131072
  %43 = xor i64 %42, 131072
  br label %update_y

update_y:                                         ; preds = %block_123, %frame
  %r_y.1 = phi i64 [ 0, %frame ], [ %44, %block_123 ]
  %44 = add nuw nsw i64 %r_y.1, 1
  %45 = shl nuw nsw i64 %r_y.1, 9
  %46 = add nuw nsw i64 %45, 130560
  %47 = and i64 %46, 130560
  %48 = shl nuw nsw i64 %44, 9
  %49 = and i64 %48, 130560
  %50 = add nuw nsw i64 %45, %42
  %51 = or disjoint i64 %47, %42
  %52 = or disjoint i64 %49, %42
  %53 = add nuw nsw i64 %45, %43
  br label %update_x

update_x:                                         ; preds = %update_x, %update_y
  %r_x.1 = phi i64 [ 0, %update_y ], [ %56, %update_x ]
  %54 = add nuw nsw i64 %r_x.1, 511
  %55 = and i64 %54, 511
  %56 = add nuw nsw i64 %r_x.1, 1
  %57 = and i64 %56, 511
  %58 = or disjoint i64 %55, %50
  %59 = or disjoint i64 %57, %50
  %60 = add nuw nsw i64 %r_x.1, %51
  %61 = add nuw nsw i64 %r_x.1, %52
  %62 = getelementptr [4 x i8], ptr %h162, i64 %58
  %63 = load float, ptr %62, align 4
  %64 = getelementptr [4 x i8], ptr %h162, i64 %59
  %65 = load float, ptr %64, align 4
  %66 = getelementptr [4 x i8], ptr %h162, i64 %60
  %67 = load float, ptr %66, align 4
  %68 = getelementptr [4 x i8], ptr %h162, i64 %61
  %69 = load float, ptr %68, align 4
  %70 = getelementptr [4 x i8], ptr %hu163, i64 %58
  %71 = load float, ptr %70, align 4
  %72 = getelementptr [4 x i8], ptr %hu163, i64 %59
  %73 = load float, ptr %72, align 4
  %74 = getelementptr [4 x i8], ptr %hu163, i64 %60
  %75 = load float, ptr %74, align 4
  %76 = getelementptr [4 x i8], ptr %hu163, i64 %61
  %77 = load float, ptr %76, align 4
  %78 = getelementptr [4 x i8], ptr %hv164, i64 %58
  %79 = load float, ptr %78, align 4
  %80 = getelementptr [4 x i8], ptr %hv164, i64 %59
  %81 = load float, ptr %80, align 4
  %82 = getelementptr [4 x i8], ptr %hv164, i64 %60
  %83 = load float, ptr %82, align 4
  %84 = getelementptr [4 x i8], ptr %hv164, i64 %61
  %85 = load float, ptr %84, align 4
  %86 = fmul float %71, %71
  %87 = fdiv float %86, %63
  %88 = fmul float %63, 5.000000e-01
  %89 = fmul float %63, %88
  %90 = fadd float %89, %87
  %91 = fmul float %73, %73
  %92 = fdiv float %91, %65
  %93 = fmul float %65, 5.000000e-01
  %94 = fmul float %65, %93
  %95 = fadd float %94, %92
  %96 = fmul float %75, %83
  %97 = fdiv float %96, %67
  %98 = fmul float %77, %85
  %99 = fdiv float %98, %69
  %100 = fmul float %71, %79
  %101 = fdiv float %100, %63
  %102 = fmul float %73, %81
  %103 = fdiv float %102, %65
  %104 = fmul float %83, %83
  %105 = fdiv float %104, %67
  %106 = fmul float %67, 5.000000e-01
  %107 = fmul float %67, %106
  %108 = fadd float %107, %105
  %109 = fmul float %85, %85
  %110 = fdiv float %109, %69
  %111 = fmul float %69, 5.000000e-01
  %112 = fmul float %69, %111
  %113 = fadd float %112, %110
  %114 = fadd float %63, %65
  %115 = fadd float %114, %67
  %116 = fadd float %115, %69
  %117 = fmul float %116, 2.500000e-01
  %118 = fadd float %71, %73
  %119 = fadd float %118, %75
  %120 = fadd float %119, %77
  %121 = fmul float %120, 2.500000e-01
  %122 = fadd float %79, %81
  %123 = fadd float %83, %122
  %124 = fadd float %85, %123
  %125 = fmul float %124, 2.500000e-01
  %126 = add nuw nsw i64 %53, %r_x.1
  %127 = fsub float %73, %71
  %128 = fsub float %85, %83
  %129 = fadd float %127, %128
  %130 = fmul float %129, 1.000000e-01
  %131 = fsub float %117, %130
  %132 = getelementptr [4 x i8], ptr %h162, i64 %126
  store float %131, ptr %132, align 4
  %133 = fsub float %95, %90
  %134 = fsub float %99, %97
  %135 = fadd float %133, %134
  %136 = fmul float %135, 1.000000e-01
  %137 = fsub float %121, %136
  %138 = getelementptr [4 x i8], ptr %hu163, i64 %126
  store float %137, ptr %138, align 4
  %139 = fsub float %103, %101
  %140 = fsub float %113, %108
  %141 = fadd float %139, %140
  %142 = fmul float %141, 1.000000e-01
  %143 = fsub float %125, %142
  %144 = getelementptr [4 x i8], ptr %hv164, i64 %126
  store float %143, ptr %144, align 4
  %145 = icmp samesign ult i64 %r_x.1, 511
  br i1 %145, label %update_x, label %block_123

block_123:                                        ; preds = %update_x
  %146 = icmp samesign ult i64 %r_y.1, 255
  br i1 %146, label %update_y, label %block_124

block_124:                                        ; preds = %block_123
  %147 = getelementptr [4 x i8], ptr %h162, i64 %43
  %148 = load float, ptr %147, align 4
  br label %find_range

find_range:                                       ; preds = %find_range, %block_124
  %f_hmin.0 = phi float [ %148, %block_124 ], [ %152, %find_range ]
  %f_hmax.0 = phi float [ %148, %block_124 ], [ %154, %find_range ]
  %r_i.0 = phi i64 [ 0, %block_124 ], [ %155, %find_range ]
  %149 = getelementptr [4 x i8], ptr %147, i64 %r_i.0
  %150 = load float, ptr %149, align 4
  %151 = fcmp olt float %150, %f_hmin.0
  %152 = select i1 %151, float %150, float %f_hmin.0
  %153 = fcmp ogt float %150, %f_hmax.0
  %154 = select i1 %153, float %150, float %f_hmax.0
  %155 = add nuw nsw i64 %r_i.0, 1
  %156 = icmp samesign ult i64 %r_i.0, 131071
  br i1 %156, label %find_range, label %block_134

block_134:                                        ; preds = %find_range
  %157 = fsub float %154, %152
  %158 = fadd float %157, f0x3089705F
  %159 = fdiv float 2.550000e+02, %158
  br label %draw_y

draw_y:                                           ; preds = %block_149, %block_134
  %r_y.2 = phi i64 [ 0, %block_134 ], [ %176, %block_149 ]
  %.lhs.trunc = trunc i64 %r_y.2 to i16
  %160 = udiv i16 %.lhs.trunc, 3
  %.zext = zext nneg i16 %160 to i64
  %.idx167 = shl nuw nsw i64 %.zext, 11
  %161 = getelementptr i8, ptr %147, i64 %.idx167
  %162 = trunc nuw nsw i64 %r_y.2 to i32
  br label %draw_x

draw_x:                                           ; preds = %draw_x, %draw_y
  %r_x.2 = phi i64 [ 0, %draw_y ], [ %174, %draw_x ]
  %.lhs.trunc172 = trunc i64 %r_x.2 to i16
  %163 = udiv i16 %.lhs.trunc172, 3
  %.zext173 = zext nneg i16 %163 to i64
  %164 = getelementptr [4 x i8], ptr %161, i64 %.zext173
  %165 = load float, ptr %164, align 4
  %166 = fsub float %165, %152
  %167 = fmul float %159, %166
  %168 = fptosi float %167 to i64
  %169 = tail call i64 @llvm.smax.i64(i64 %168, i64 0)
  %170 = tail call i64 @llvm.umin.i64(i64 %169, i64 255)
  %171 = trunc nuw nsw i64 %r_x.2 to i32
  %172 = trunc nuw nsw i64 %170 to i32
  %173 = or disjoint i32 %172, -16777216
  tail call void @simPutPixel(i32 %171, i32 %162, i32 %173)
  %174 = add nuw nsw i64 %r_x.2, 1
  %175 = icmp samesign ult i64 %r_x.2, 1535
  br i1 %175, label %draw_x, label %block_149

block_149:                                        ; preds = %draw_x
  %176 = add nuw nsw i64 %r_y.2, 1
  %177 = icmp samesign ult i64 %r_y.2, 767
  br i1 %177, label %draw_y, label %block_150

block_150:                                        ; preds = %block_149
  tail call void @simFlush()
  %178 = add nuw nsw i64 %r_step.0, 1
  %179 = icmp samesign ult i64 %r_step.0, 19999
  br i1 %179, label %frame, label %block_152

block_152:                                        ; preds = %block_150
  ret void
}

declare void @simPutPixel(i32, i32, i32) local_unnamed_addr

declare void @simFlush() local_unnamed_addr

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #0

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umin.i64(i64, i64) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #1

attributes #0 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: write) }
