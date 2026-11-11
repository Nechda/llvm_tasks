; ModuleID = 'assembly'
source_filename = "assembly"

define void @app() {
allocations:
  %r_y = alloca i64, align 8
  %r_row = alloca i64, align 8
  %r_x = alloca i64, align 8
  %f_bump = alloca float, align 4
  %r_b = alloca i64, align 8
  %c_top = alloca i1, align 1
  %c_left = alloca i1, align 1
  %r_tx = alloca i64, align 8
  %r_ty = alloca i64, align 8
  %r_dx = alloca i64, align 8
  %r_dy = alloca i64, align 8
  %r_dx2 = alloca i64, align 8
  %r_dy2 = alloca i64, align 8
  %r_d2 = alloca i64, align 8
  %c_inside = alloca i1, align 1
  %f_d2 = alloca float, align 4
  %f_add = alloca float, align 4
  %c_higher = alloca i1, align 1
  %f_height = alloca float, align 4
  %r_idx = alloca i64, align 8
  %r_step = alloca i64, align 8
  %r_cur = alloca i64, align 8
  %r_nxt = alloca i64, align 8
  %r_src = alloca i64, align 8
  %r_dst = alloca i64, align 8
  %r_jm = alloca i64, align 8
  %r_jp = alloca i64, align 8
  %r_rowm = alloca i64, align 8
  %r_rowp = alloca i64, align 8
  %r_im = alloca i64, align 8
  %r_ip = alloca i64, align 8
  %r_left = alloca i64, align 8
  %r_right = alloca i64, align 8
  %r_down = alloca i64, align 8
  %r_up = alloca i64, align 8
  %f_hm = alloca float, align 4
  %f_hp = alloca float, align 4
  %f_hbw = alloca float, align 4
  %f_hup = alloca float, align 4
  %f_um = alloca float, align 4
  %f_up = alloca float, align 4
  %f_ubw = alloca float, align 4
  %f_uup = alloca float, align 4
  %f_vm = alloca float, align 4
  %f_vp = alloca float, align 4
  %f_vbw = alloca float, align 4
  %f_vup = alloca float, align 4
  %f_t = alloca float, align 4
  %f_half = alloca float, align 4
  %f_fxm = alloca float, align 4
  %f_fxp = alloca float, align 4
  %f_fym = alloca float, align 4
  %f_fyp = alloca float, align 4
  %f_gxm = alloca float, align 4
  %f_gxp = alloca float, align 4
  %f_gym = alloca float, align 4
  %f_gyp = alloca float, align 4
  %f_hav = alloca float, align 4
  %f_uav = alloca float, align 4
  %f_vav = alloca float, align 4
  %f_a = alloca float, align 4
  %f_bv = alloca float, align 4
  %f_result = alloca float, align 4
  %f_hmin = alloca float, align 4
  %f_hmax = alloca float, align 4
  %r_i = alloca i64, align 8
  %c_lower = alloca i1, align 1
  %f_inv = alloca float, align 4
  %r_j = alloca i64, align 8
  %r_brightness = alloca i64, align 8
  %r_color = alloca i64, align 8
  %h = alloca float, i64 262144, align 4
  %hu = alloca float, i64 262144, align 4
  %hv = alloca float, i64 262144, align 4
  br label %entry

entry:                                            ; preds = %allocations
  br label %fill.test

init_y:                                           ; preds = %block_30, %fill.done3
  %0 = load i64, ptr %r_y, align 4
  %1 = mul i64 %0, 512
  store i64 %1, ptr %r_row, align 4
  store i64 0, ptr %r_x, align 4
  br label %init_x

init_x:                                           ; preds = %block_26, %init_y
  store float 0.000000e+00, ptr %f_bump, align 4
  store i64 0, ptr %r_b, align 4
  br label %init_bump

init_bump:                                        ; preds = %bump_next, %init_x
  %2 = load i64, ptr %r_b, align 4
  %3 = icmp eq i64 %2, 0
  store i1 %3, ptr %c_top, align 1
  %4 = load i64, ptr %r_b, align 4
  %5 = icmp eq i64 %4, 1
  store i1 %5, ptr %c_left, align 1
  %6 = load i1, ptr %c_left, align 1
  %7 = select i1 %6, i64 205, i64 307
  store i64 %7, ptr %r_tx, align 4
  %8 = load i1, ptr %c_top, align 1
  %9 = load i64, ptr %r_tx, align 4
  %10 = select i1 %8, i64 256, i64 %9
  store i64 %10, ptr %r_tx, align 4
  %11 = load i1, ptr %c_top, align 1
  %12 = select i1 %11, i64 68, i64 158
  store i64 %12, ptr %r_ty, align 4
  %13 = load i64, ptr %r_x, align 4
  %14 = load i64, ptr %r_tx, align 4
  %15 = sub i64 %13, %14
  store i64 %15, ptr %r_dx, align 4
  %16 = load i64, ptr %r_y, align 4
  %17 = load i64, ptr %r_ty, align 4
  %18 = sub i64 %16, %17
  store i64 %18, ptr %r_dy, align 4
  %19 = load i64, ptr %r_dx, align 4
  %20 = load i64, ptr %r_dx, align 4
  %21 = mul i64 %19, %20
  store i64 %21, ptr %r_dx2, align 4
  %22 = load i64, ptr %r_dy, align 4
  %23 = load i64, ptr %r_dy, align 4
  %24 = mul i64 %22, %23
  store i64 %24, ptr %r_dy2, align 4
  %25 = load i64, ptr %r_dy2, align 4
  %26 = load i64, ptr %r_dx2, align 4
  %27 = add i64 %25, %26
  store i64 %27, ptr %r_d2, align 4
  %28 = load i64, ptr %r_d2, align 4
  %29 = icmp slt i64 %28, 400
  store i1 %29, ptr %c_inside, align 1
  %30 = load i1, ptr %c_inside, align 1
  %31 = icmp ne i1 %30, false
  br i1 %31, label %bump_inside, label %bump_next

bump_inside:                                      ; preds = %init_bump
  %32 = load i64, ptr %r_d2, align 4
  %33 = sitofp i64 %32 to float
  store float %33, ptr %f_d2, align 4
  %34 = load float, ptr %f_d2, align 4
  %35 = fdiv float %34, 4.000000e+02
  store float %35, ptr %f_add, align 4
  %36 = load float, ptr %f_add, align 4
  %37 = fsub float 1.000000e+00, %36
  store float %37, ptr %f_add, align 4
  %38 = load float, ptr %f_add, align 4
  %39 = fmul float %38, 5.000000e-01
  store float %39, ptr %f_add, align 4
  %40 = load float, ptr %f_add, align 4
  %41 = load float, ptr %f_bump, align 4
  %42 = fcmp ogt float %40, %41
  store i1 %42, ptr %c_higher, align 1
  %43 = load i1, ptr %c_higher, align 1
  %44 = load float, ptr %f_add, align 4
  %45 = load float, ptr %f_bump, align 4
  %46 = select i1 %43, float %44, float %45
  store float %46, ptr %f_bump, align 4
  br label %bump_next

bump_next:                                        ; preds = %bump_inside, %init_bump
  %47 = load i64, ptr %r_b, align 4
  %48 = add i64 %47, 1
  store i64 %48, ptr %r_b, align 4
  %49 = icmp slt i64 %48, 3
  br i1 %49, label %init_bump, label %block_26

block_26:                                         ; preds = %bump_next
  %50 = load float, ptr %f_bump, align 4
  %51 = fadd float %50, 1.000000e+00
  store float %51, ptr %f_height, align 4
  %52 = load i64, ptr %r_row, align 4
  %53 = load i64, ptr %r_x, align 4
  %54 = add i64 %52, %53
  store i64 %54, ptr %r_idx, align 4
  %55 = load i64, ptr %r_idx, align 4
  %56 = getelementptr float, ptr %h, i64 %55
  %57 = load float, ptr %f_height, align 4
  store float %57, ptr %56, align 4
  %58 = load i64, ptr %r_x, align 4
  %59 = add i64 %58, 1
  store i64 %59, ptr %r_x, align 4
  %60 = icmp slt i64 %59, 512
  br i1 %60, label %init_x, label %block_30

block_30:                                         ; preds = %block_26
  %61 = load i64, ptr %r_y, align 4
  %62 = add i64 %61, 1
  store i64 %62, ptr %r_y, align 4
  %63 = icmp slt i64 %62, 256
  br i1 %63, label %init_y, label %block_31

block_31:                                         ; preds = %block_30
  store i64 0, ptr %r_step, align 4
  br label %frame

frame:                                            ; preds = %block_150, %block_31
  %64 = load i64, ptr %r_step, align 4
  %65 = and i64 %64, 1
  store i64 %65, ptr %r_cur, align 4
  %66 = load i64, ptr %r_cur, align 4
  %67 = xor i64 %66, 1
  store i64 %67, ptr %r_nxt, align 4
  %68 = load i64, ptr %r_cur, align 4
  %69 = mul i64 %68, 131072
  store i64 %69, ptr %r_src, align 4
  %70 = load i64, ptr %r_nxt, align 4
  %71 = mul i64 %70, 131072
  store i64 %71, ptr %r_dst, align 4
  store i64 0, ptr %r_y, align 4
  br label %update_y

update_y:                                         ; preds = %block_123, %frame
  %72 = load i64, ptr %r_y, align 4
  %73 = sub i64 %72, 1
  store i64 %73, ptr %r_jm, align 4
  %74 = load i64, ptr %r_jm, align 4
  %75 = and i64 %74, 255
  store i64 %75, ptr %r_jm, align 4
  %76 = load i64, ptr %r_y, align 4
  %77 = add i64 %76, 1
  store i64 %77, ptr %r_jp, align 4
  %78 = load i64, ptr %r_jp, align 4
  %79 = and i64 %78, 255
  store i64 %79, ptr %r_jp, align 4
  %80 = load i64, ptr %r_y, align 4
  %81 = mul i64 %80, 512
  store i64 %81, ptr %r_row, align 4
  %82 = load i64, ptr %r_jm, align 4
  %83 = mul i64 %82, 512
  store i64 %83, ptr %r_rowm, align 4
  %84 = load i64, ptr %r_jp, align 4
  %85 = mul i64 %84, 512
  store i64 %85, ptr %r_rowp, align 4
  %86 = load i64, ptr %r_src, align 4
  %87 = load i64, ptr %r_row, align 4
  %88 = add i64 %86, %87
  store i64 %88, ptr %r_row, align 4
  %89 = load i64, ptr %r_src, align 4
  %90 = load i64, ptr %r_rowm, align 4
  %91 = add i64 %89, %90
  store i64 %91, ptr %r_rowm, align 4
  %92 = load i64, ptr %r_src, align 4
  %93 = load i64, ptr %r_rowp, align 4
  %94 = add i64 %92, %93
  store i64 %94, ptr %r_rowp, align 4
  store i64 0, ptr %r_x, align 4
  br label %update_x

update_x:                                         ; preds = %update_x, %update_y
  %95 = load i64, ptr %r_x, align 4
  %96 = sub i64 %95, 1
  store i64 %96, ptr %r_im, align 4
  %97 = load i64, ptr %r_im, align 4
  %98 = and i64 %97, 511
  store i64 %98, ptr %r_im, align 4
  %99 = load i64, ptr %r_x, align 4
  %100 = add i64 %99, 1
  store i64 %100, ptr %r_ip, align 4
  %101 = load i64, ptr %r_ip, align 4
  %102 = and i64 %101, 511
  store i64 %102, ptr %r_ip, align 4
  %103 = load i64, ptr %r_row, align 4
  %104 = load i64, ptr %r_im, align 4
  %105 = add i64 %103, %104
  store i64 %105, ptr %r_left, align 4
  %106 = load i64, ptr %r_row, align 4
  %107 = load i64, ptr %r_ip, align 4
  %108 = add i64 %106, %107
  store i64 %108, ptr %r_right, align 4
  %109 = load i64, ptr %r_rowm, align 4
  %110 = load i64, ptr %r_x, align 4
  %111 = add i64 %109, %110
  store i64 %111, ptr %r_down, align 4
  %112 = load i64, ptr %r_rowp, align 4
  %113 = load i64, ptr %r_x, align 4
  %114 = add i64 %112, %113
  store i64 %114, ptr %r_up, align 4
  %115 = load i64, ptr %r_left, align 4
  %116 = getelementptr float, ptr %h, i64 %115
  %117 = load float, ptr %116, align 4
  store float %117, ptr %f_hm, align 4
  %118 = load i64, ptr %r_right, align 4
  %119 = getelementptr float, ptr %h, i64 %118
  %120 = load float, ptr %119, align 4
  store float %120, ptr %f_hp, align 4
  %121 = load i64, ptr %r_down, align 4
  %122 = getelementptr float, ptr %h, i64 %121
  %123 = load float, ptr %122, align 4
  store float %123, ptr %f_hbw, align 4
  %124 = load i64, ptr %r_up, align 4
  %125 = getelementptr float, ptr %h, i64 %124
  %126 = load float, ptr %125, align 4
  store float %126, ptr %f_hup, align 4
  %127 = load i64, ptr %r_left, align 4
  %128 = getelementptr float, ptr %hu, i64 %127
  %129 = load float, ptr %128, align 4
  store float %129, ptr %f_um, align 4
  %130 = load i64, ptr %r_right, align 4
  %131 = getelementptr float, ptr %hu, i64 %130
  %132 = load float, ptr %131, align 4
  store float %132, ptr %f_up, align 4
  %133 = load i64, ptr %r_down, align 4
  %134 = getelementptr float, ptr %hu, i64 %133
  %135 = load float, ptr %134, align 4
  store float %135, ptr %f_ubw, align 4
  %136 = load i64, ptr %r_up, align 4
  %137 = getelementptr float, ptr %hu, i64 %136
  %138 = load float, ptr %137, align 4
  store float %138, ptr %f_uup, align 4
  %139 = load i64, ptr %r_left, align 4
  %140 = getelementptr float, ptr %hv, i64 %139
  %141 = load float, ptr %140, align 4
  store float %141, ptr %f_vm, align 4
  %142 = load i64, ptr %r_right, align 4
  %143 = getelementptr float, ptr %hv, i64 %142
  %144 = load float, ptr %143, align 4
  store float %144, ptr %f_vp, align 4
  %145 = load i64, ptr %r_down, align 4
  %146 = getelementptr float, ptr %hv, i64 %145
  %147 = load float, ptr %146, align 4
  store float %147, ptr %f_vbw, align 4
  %148 = load i64, ptr %r_up, align 4
  %149 = getelementptr float, ptr %hv, i64 %148
  %150 = load float, ptr %149, align 4
  store float %150, ptr %f_vup, align 4
  %151 = load float, ptr %f_um, align 4
  %152 = load float, ptr %f_um, align 4
  %153 = fmul float %151, %152
  store float %153, ptr %f_t, align 4
  %154 = load float, ptr %f_t, align 4
  %155 = load float, ptr %f_hm, align 4
  %156 = fdiv float %154, %155
  store float %156, ptr %f_t, align 4
  %157 = load float, ptr %f_hm, align 4
  %158 = fmul float %157, 5.000000e-01
  store float %158, ptr %f_half, align 4
  %159 = load float, ptr %f_half, align 4
  %160 = load float, ptr %f_hm, align 4
  %161 = load float, ptr %f_t, align 4
  %162 = fmul float %159, %160
  %163 = fadd float %162, %161
  store float %163, ptr %f_fxm, align 4
  %164 = load float, ptr %f_up, align 4
  %165 = load float, ptr %f_up, align 4
  %166 = fmul float %164, %165
  store float %166, ptr %f_t, align 4
  %167 = load float, ptr %f_t, align 4
  %168 = load float, ptr %f_hp, align 4
  %169 = fdiv float %167, %168
  store float %169, ptr %f_t, align 4
  %170 = load float, ptr %f_hp, align 4
  %171 = fmul float %170, 5.000000e-01
  store float %171, ptr %f_half, align 4
  %172 = load float, ptr %f_half, align 4
  %173 = load float, ptr %f_hp, align 4
  %174 = load float, ptr %f_t, align 4
  %175 = fmul float %172, %173
  %176 = fadd float %175, %174
  store float %176, ptr %f_fxp, align 4
  %177 = load float, ptr %f_vbw, align 4
  %178 = load float, ptr %f_ubw, align 4
  %179 = fmul float %177, %178
  store float %179, ptr %f_t, align 4
  %180 = load float, ptr %f_t, align 4
  %181 = load float, ptr %f_hbw, align 4
  %182 = fdiv float %180, %181
  store float %182, ptr %f_fym, align 4
  %183 = load float, ptr %f_vup, align 4
  %184 = load float, ptr %f_uup, align 4
  %185 = fmul float %183, %184
  store float %185, ptr %f_t, align 4
  %186 = load float, ptr %f_t, align 4
  %187 = load float, ptr %f_hup, align 4
  %188 = fdiv float %186, %187
  store float %188, ptr %f_fyp, align 4
  %189 = load float, ptr %f_um, align 4
  %190 = load float, ptr %f_vm, align 4
  %191 = fmul float %189, %190
  store float %191, ptr %f_t, align 4
  %192 = load float, ptr %f_t, align 4
  %193 = load float, ptr %f_hm, align 4
  %194 = fdiv float %192, %193
  store float %194, ptr %f_gxm, align 4
  %195 = load float, ptr %f_up, align 4
  %196 = load float, ptr %f_vp, align 4
  %197 = fmul float %195, %196
  store float %197, ptr %f_t, align 4
  %198 = load float, ptr %f_t, align 4
  %199 = load float, ptr %f_hp, align 4
  %200 = fdiv float %198, %199
  store float %200, ptr %f_gxp, align 4
  %201 = load float, ptr %f_vbw, align 4
  %202 = load float, ptr %f_vbw, align 4
  %203 = fmul float %201, %202
  store float %203, ptr %f_t, align 4
  %204 = load float, ptr %f_t, align 4
  %205 = load float, ptr %f_hbw, align 4
  %206 = fdiv float %204, %205
  store float %206, ptr %f_t, align 4
  %207 = load float, ptr %f_hbw, align 4
  %208 = fmul float %207, 5.000000e-01
  store float %208, ptr %f_half, align 4
  %209 = load float, ptr %f_half, align 4
  %210 = load float, ptr %f_hbw, align 4
  %211 = load float, ptr %f_t, align 4
  %212 = fmul float %209, %210
  %213 = fadd float %212, %211
  store float %213, ptr %f_gym, align 4
  %214 = load float, ptr %f_vup, align 4
  %215 = load float, ptr %f_vup, align 4
  %216 = fmul float %214, %215
  store float %216, ptr %f_t, align 4
  %217 = load float, ptr %f_t, align 4
  %218 = load float, ptr %f_hup, align 4
  %219 = fdiv float %217, %218
  store float %219, ptr %f_t, align 4
  %220 = load float, ptr %f_hup, align 4
  %221 = fmul float %220, 5.000000e-01
  store float %221, ptr %f_half, align 4
  %222 = load float, ptr %f_half, align 4
  %223 = load float, ptr %f_hup, align 4
  %224 = load float, ptr %f_t, align 4
  %225 = fmul float %222, %223
  %226 = fadd float %225, %224
  store float %226, ptr %f_gyp, align 4
  %227 = load float, ptr %f_hm, align 4
  %228 = load float, ptr %f_hp, align 4
  %229 = fadd float %227, %228
  store float %229, ptr %f_hav, align 4
  %230 = load float, ptr %f_hav, align 4
  %231 = load float, ptr %f_hbw, align 4
  %232 = fadd float %230, %231
  store float %232, ptr %f_hav, align 4
  %233 = load float, ptr %f_hav, align 4
  %234 = load float, ptr %f_hup, align 4
  %235 = fadd float %233, %234
  store float %235, ptr %f_hav, align 4
  %236 = load float, ptr %f_hav, align 4
  %237 = fmul float %236, 2.500000e-01
  store float %237, ptr %f_hav, align 4
  %238 = load float, ptr %f_um, align 4
  %239 = load float, ptr %f_up, align 4
  %240 = fadd float %238, %239
  store float %240, ptr %f_uav, align 4
  %241 = load float, ptr %f_uav, align 4
  %242 = load float, ptr %f_ubw, align 4
  %243 = fadd float %241, %242
  store float %243, ptr %f_uav, align 4
  %244 = load float, ptr %f_uav, align 4
  %245 = load float, ptr %f_uup, align 4
  %246 = fadd float %244, %245
  store float %246, ptr %f_uav, align 4
  %247 = load float, ptr %f_uav, align 4
  %248 = fmul float %247, 2.500000e-01
  store float %248, ptr %f_uav, align 4
  %249 = load float, ptr %f_vm, align 4
  %250 = load float, ptr %f_vp, align 4
  %251 = fadd float %249, %250
  store float %251, ptr %f_vav, align 4
  %252 = load float, ptr %f_vbw, align 4
  %253 = load float, ptr %f_vav, align 4
  %254 = fadd float %252, %253
  store float %254, ptr %f_vav, align 4
  %255 = load float, ptr %f_vup, align 4
  %256 = load float, ptr %f_vav, align 4
  %257 = fadd float %255, %256
  store float %257, ptr %f_vav, align 4
  %258 = load float, ptr %f_vav, align 4
  %259 = fmul float %258, 2.500000e-01
  store float %259, ptr %f_vav, align 4
  %260 = load i64, ptr %r_y, align 4
  %261 = mul i64 %260, 512
  store i64 %261, ptr %r_idx, align 4
  %262 = load i64, ptr %r_idx, align 4
  %263 = load i64, ptr %r_x, align 4
  %264 = add i64 %262, %263
  store i64 %264, ptr %r_idx, align 4
  %265 = load i64, ptr %r_dst, align 4
  %266 = load i64, ptr %r_idx, align 4
  %267 = add i64 %265, %266
  store i64 %267, ptr %r_idx, align 4
  %268 = load float, ptr %f_up, align 4
  %269 = load float, ptr %f_um, align 4
  %270 = fsub float %268, %269
  store float %270, ptr %f_a, align 4
  %271 = load float, ptr %f_vup, align 4
  %272 = load float, ptr %f_vbw, align 4
  %273 = fsub float %271, %272
  store float %273, ptr %f_bv, align 4
  %274 = load float, ptr %f_a, align 4
  %275 = load float, ptr %f_bv, align 4
  %276 = fadd float %274, %275
  store float %276, ptr %f_a, align 4
  %277 = load float, ptr %f_a, align 4
  %278 = load float, ptr %f_hav, align 4
  %279 = fmul float %277, -1.000000e-01
  %280 = fadd float %279, %278
  store float %280, ptr %f_result, align 4
  %281 = load i64, ptr %r_idx, align 4
  %282 = getelementptr float, ptr %h, i64 %281
  %283 = load float, ptr %f_result, align 4
  store float %283, ptr %282, align 4
  %284 = load float, ptr %f_fxp, align 4
  %285 = load float, ptr %f_fxm, align 4
  %286 = fsub float %284, %285
  store float %286, ptr %f_a, align 4
  %287 = load float, ptr %f_fyp, align 4
  %288 = load float, ptr %f_fym, align 4
  %289 = fsub float %287, %288
  store float %289, ptr %f_bv, align 4
  %290 = load float, ptr %f_a, align 4
  %291 = load float, ptr %f_bv, align 4
  %292 = fadd float %290, %291
  store float %292, ptr %f_a, align 4
  %293 = load float, ptr %f_a, align 4
  %294 = load float, ptr %f_uav, align 4
  %295 = fmul float %293, -1.000000e-01
  %296 = fadd float %295, %294
  store float %296, ptr %f_result, align 4
  %297 = load i64, ptr %r_idx, align 4
  %298 = getelementptr float, ptr %hu, i64 %297
  %299 = load float, ptr %f_result, align 4
  store float %299, ptr %298, align 4
  %300 = load float, ptr %f_gxp, align 4
  %301 = load float, ptr %f_gxm, align 4
  %302 = fsub float %300, %301
  store float %302, ptr %f_a, align 4
  %303 = load float, ptr %f_gyp, align 4
  %304 = load float, ptr %f_gym, align 4
  %305 = fsub float %303, %304
  store float %305, ptr %f_bv, align 4
  %306 = load float, ptr %f_a, align 4
  %307 = load float, ptr %f_bv, align 4
  %308 = fadd float %306, %307
  store float %308, ptr %f_a, align 4
  %309 = load float, ptr %f_a, align 4
  %310 = load float, ptr %f_vav, align 4
  %311 = fmul float %309, -1.000000e-01
  %312 = fadd float %311, %310
  store float %312, ptr %f_result, align 4
  %313 = load i64, ptr %r_idx, align 4
  %314 = getelementptr float, ptr %hv, i64 %313
  %315 = load float, ptr %f_result, align 4
  store float %315, ptr %314, align 4
  %316 = load i64, ptr %r_x, align 4
  %317 = add i64 %316, 1
  store i64 %317, ptr %r_x, align 4
  %318 = icmp slt i64 %317, 512
  br i1 %318, label %update_x, label %block_123

block_123:                                        ; preds = %update_x
  %319 = load i64, ptr %r_y, align 4
  %320 = add i64 %319, 1
  store i64 %320, ptr %r_y, align 4
  %321 = icmp slt i64 %320, 256
  br i1 %321, label %update_y, label %block_124

block_124:                                        ; preds = %block_123
  %322 = load i64, ptr %r_dst, align 4
  %323 = getelementptr float, ptr %h, i64 %322
  %324 = load float, ptr %323, align 4
  store float %324, ptr %f_hmin, align 4
  %325 = load float, ptr %f_hmin, align 4
  store float %325, ptr %f_hmax, align 4
  store i64 0, ptr %r_i, align 4
  br label %find_range

find_range:                                       ; preds = %find_range, %block_124
  %326 = load i64, ptr %r_dst, align 4
  %327 = load i64, ptr %r_i, align 4
  %328 = add i64 %326, %327
  store i64 %328, ptr %r_idx, align 4
  %329 = load i64, ptr %r_idx, align 4
  %330 = getelementptr float, ptr %h, i64 %329
  %331 = load float, ptr %330, align 4
  store float %331, ptr %f_height, align 4
  %332 = load float, ptr %f_height, align 4
  %333 = load float, ptr %f_hmin, align 4
  %334 = fcmp olt float %332, %333
  store i1 %334, ptr %c_lower, align 1
  %335 = load i1, ptr %c_lower, align 1
  %336 = load float, ptr %f_height, align 4
  %337 = load float, ptr %f_hmin, align 4
  %338 = select i1 %335, float %336, float %337
  store float %338, ptr %f_hmin, align 4
  %339 = load float, ptr %f_height, align 4
  %340 = load float, ptr %f_hmax, align 4
  %341 = fcmp ogt float %339, %340
  store i1 %341, ptr %c_higher, align 1
  %342 = load i1, ptr %c_higher, align 1
  %343 = load float, ptr %f_height, align 4
  %344 = load float, ptr %f_hmax, align 4
  %345 = select i1 %342, float %343, float %344
  store float %345, ptr %f_hmax, align 4
  %346 = load i64, ptr %r_i, align 4
  %347 = add i64 %346, 1
  store i64 %347, ptr %r_i, align 4
  %348 = icmp slt i64 %347, 131072
  br i1 %348, label %find_range, label %block_134

block_134:                                        ; preds = %find_range
  %349 = load float, ptr %f_hmax, align 4
  %350 = load float, ptr %f_hmin, align 4
  %351 = fsub float %349, %350
  store float %351, ptr %f_inv, align 4
  %352 = load float, ptr %f_inv, align 4
  %353 = fadd float %352, f0x3089705F
  store float %353, ptr %f_inv, align 4
  %354 = load float, ptr %f_inv, align 4
  %355 = fdiv float 2.550000e+02, %354
  store float %355, ptr %f_inv, align 4
  store i64 0, ptr %r_y, align 4
  br label %draw_y

draw_y:                                           ; preds = %block_149, %block_134
  %356 = load i64, ptr %r_y, align 4
  %357 = udiv i64 %356, 3
  store i64 %357, ptr %r_j, align 4
  %358 = load i64, ptr %r_j, align 4
  %359 = mul i64 %358, 512
  store i64 %359, ptr %r_row, align 4
  %360 = load i64, ptr %r_dst, align 4
  %361 = load i64, ptr %r_row, align 4
  %362 = add i64 %360, %361
  store i64 %362, ptr %r_row, align 4
  store i64 0, ptr %r_x, align 4
  br label %draw_x

draw_x:                                           ; preds = %draw_x, %draw_y
  %363 = load i64, ptr %r_x, align 4
  %364 = udiv i64 %363, 3
  store i64 %364, ptr %r_i, align 4
  %365 = load i64, ptr %r_row, align 4
  %366 = load i64, ptr %r_i, align 4
  %367 = add i64 %365, %366
  store i64 %367, ptr %r_idx, align 4
  %368 = load i64, ptr %r_idx, align 4
  %369 = getelementptr float, ptr %h, i64 %368
  %370 = load float, ptr %369, align 4
  store float %370, ptr %f_height, align 4
  %371 = load float, ptr %f_height, align 4
  %372 = load float, ptr %f_hmin, align 4
  %373 = load float, ptr %f_inv, align 4
  %374 = fsub float %371, %372
  %375 = fmul float %374, %373
  %376 = fptosi float %375 to i64
  %377 = icmp slt i64 %376, 0
  %378 = select i1 %377, i64 0, i64 %376
  %379 = icmp sgt i64 %378, 255
  %380 = select i1 %379, i64 255, i64 %378
  store i64 %380, ptr %r_brightness, align 4
  %381 = load i64, ptr %r_brightness, align 4
  %382 = or i64 %381, 4278190080
  store i64 %382, ptr %r_color, align 4
  %383 = load i64, ptr %r_x, align 4
  %384 = load i64, ptr %r_y, align 4
  %385 = load i64, ptr %r_color, align 4
  %386 = trunc i64 %383 to i32
  %387 = trunc i64 %384 to i32
  %388 = trunc i64 %385 to i32
  call void @simPutPixel(i32 %386, i32 %387, i32 %388)
  %389 = load i64, ptr %r_x, align 4
  %390 = add i64 %389, 1
  store i64 %390, ptr %r_x, align 4
  %391 = icmp slt i64 %390, 1536
  br i1 %391, label %draw_x, label %block_149

block_149:                                        ; preds = %draw_x
  %392 = load i64, ptr %r_y, align 4
  %393 = add i64 %392, 1
  store i64 %393, ptr %r_y, align 4
  %394 = icmp slt i64 %393, 768
  br i1 %394, label %draw_y, label %block_150

block_150:                                        ; preds = %block_149
  call void @simFlush()
  %395 = load i64, ptr %r_step, align 4
  %396 = add i64 %395, 1
  store i64 %396, ptr %r_step, align 4
  %397 = icmp slt i64 %396, 20000
  br i1 %397, label %frame, label %block_152

block_152:                                        ; preds = %block_150
  ret void

fill.test:                                        ; preds = %fill.body, %entry
  %398 = phi i64 [ 0, %entry ], [ %402, %fill.body ]
  %399 = icmp ult i64 %398, 131072
  br i1 %399, label %fill.body, label %fill.done

fill.body:                                        ; preds = %fill.test
  %400 = add i64 0, %398
  %401 = getelementptr float, ptr %hu, i64 %400
  store float 0.000000e+00, ptr %401, align 4
  %402 = add i64 %398, 1
  br label %fill.test

fill.done:                                        ; preds = %fill.test
  br label %fill.test1

fill.test1:                                       ; preds = %fill.body2, %fill.done
  %403 = phi i64 [ 0, %fill.done ], [ %407, %fill.body2 ]
  %404 = icmp ult i64 %403, 131072
  br i1 %404, label %fill.body2, label %fill.done3

fill.body2:                                       ; preds = %fill.test1
  %405 = add i64 0, %403
  %406 = getelementptr float, ptr %hv, i64 %405
  store float 0.000000e+00, ptr %406, align 4
  %407 = add i64 %403, 1
  br label %fill.test1

fill.done3:                                       ; preds = %fill.test1
  store i64 0, ptr %r_y, align 4
  br label %init_y
}

declare void @simPutPixel(i32, i32, i32)

declare void @simFlush()
