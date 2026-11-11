; Scalar translation of task_1/app_Os.ir. Indices count float32 elements.
.const NX, 512
.const NY, 256
.const PLANE, 131072
.const FRAMES, 20000
.buffer h, f32, 262144
.buffer hu, f32, 262144
.buffer hv, f32, 262144

entry:
    fill.f32 hu, 0, PLANE, 0.0
    fill.f32 hv, 0, PLANE, 0.0
    mov.i64 r_y, 0
init_y:
    mul.i64 r_row, r_y, NX
    mov.i64 r_x, 0
init_x:
    mov.f32 f_bump, 0.0
    mov.i64 r_b, 0
init_bump:
    cmp.eq.i64 c_top, r_b, 0
    cmp.eq.i64 c_left, r_b, 1
    sel.i64 r_tx, c_left, 205, 307
    sel.i64 r_tx, c_top, 256, r_tx
    sel.i64 r_ty, c_top, 68, 158
    sub.i64 r_dx, r_x, r_tx
    sub.i64 r_dy, r_y, r_ty
    mul.i64 r_dx2, r_dx, r_dx
    mul.i64 r_dy2, r_dy, r_dy
    add.i64 r_d2, r_dy2, r_dx2
    cmp.lt.i64 c_inside, r_d2, 400
    br c_inside, bump_inside, bump_next
bump_inside:
    itof.f32 f_d2, r_d2
    div.f32 f_add, f_d2, 400.0
    sub.f32 f_add, 1.0, f_add
    mul.f32 f_add, f_add, 0.5
    cmp.gt.f32 c_higher, f_add, f_bump
    sel.f32 f_bump, c_higher, f_add, f_bump
bump_next:
    loop r_b, 1, 3, init_bump
    add.f32 f_height, f_bump, 1.0
    add.i64 r_idx, r_row, r_x
    st.f32 [h + r_idx], f_height
    loop r_x, 1, NX, init_x
    loop r_y, 1, NY, init_y

    mov.i64 r_step, 0
frame:
    and.i64 r_cur, r_step, 1
    xor.i64 r_nxt, r_cur, 1
    mul.i64 r_src, r_cur, PLANE
    mul.i64 r_dst, r_nxt, PLANE
    mov.i64 r_y, 0
update_y:
    sub.i64 r_jm, r_y, 1
    and.i64 r_jm, r_jm, 255
    add.i64 r_jp, r_y, 1
    and.i64 r_jp, r_jp, 255
    mul.i64 r_row, r_y, NX
    mul.i64 r_rowm, r_jm, NX
    mul.i64 r_rowp, r_jp, NX
    add.i64 r_row, r_src, r_row
    add.i64 r_rowm, r_src, r_rowm
    add.i64 r_rowp, r_src, r_rowp
    mov.i64 r_x, 0
update_x:
    sub.i64 r_im, r_x, 1
    and.i64 r_im, r_im, 511
    add.i64 r_ip, r_x, 1
    and.i64 r_ip, r_ip, 511
    add.i64 r_left, r_row, r_im
    add.i64 r_right, r_row, r_ip
    add.i64 r_down, r_rowm, r_x
    add.i64 r_up, r_rowp, r_x
    ld.f32 f_hm, [h + r_left]
    ld.f32 f_hp, [h + r_right]
    ld.f32 f_hbw, [h + r_down]
    ld.f32 f_hup, [h + r_up]
    ld.f32 f_um, [hu + r_left]
    ld.f32 f_up, [hu + r_right]
    ld.f32 f_ubw, [hu + r_down]
    ld.f32 f_uup, [hu + r_up]
    ld.f32 f_vm, [hv + r_left]
    ld.f32 f_vp, [hv + r_right]
    ld.f32 f_vbw, [hv + r_down]
    ld.f32 f_vup, [hv + r_up]

    mul.f32 f_t, f_um, f_um
    div.f32 f_t, f_t, f_hm
    mul.f32 f_half, f_hm, 0.5
    madd.f32 f_fxm, f_half, f_hm, f_t
    mul.f32 f_t, f_up, f_up
    div.f32 f_t, f_t, f_hp
    mul.f32 f_half, f_hp, 0.5
    madd.f32 f_fxp, f_half, f_hp, f_t
    mul.f32 f_t, f_vbw, f_ubw
    div.f32 f_fym, f_t, f_hbw
    mul.f32 f_t, f_vup, f_uup
    div.f32 f_fyp, f_t, f_hup

    mul.f32 f_t, f_um, f_vm
    div.f32 f_gxm, f_t, f_hm
    mul.f32 f_t, f_up, f_vp
    div.f32 f_gxp, f_t, f_hp
    mul.f32 f_t, f_vbw, f_vbw
    div.f32 f_t, f_t, f_hbw
    mul.f32 f_half, f_hbw, 0.5
    madd.f32 f_gym, f_half, f_hbw, f_t
    mul.f32 f_t, f_vup, f_vup
    div.f32 f_t, f_t, f_hup
    mul.f32 f_half, f_hup, 0.5
    madd.f32 f_gyp, f_half, f_hup, f_t

    add.f32 f_hav, f_hm, f_hp
    add.f32 f_hav, f_hav, f_hbw
    add.f32 f_hav, f_hav, f_hup
    mul.f32 f_hav, f_hav, 0.25
    add.f32 f_uav, f_um, f_up
    add.f32 f_uav, f_uav, f_ubw
    add.f32 f_uav, f_uav, f_uup
    mul.f32 f_uav, f_uav, 0.25
    add.f32 f_vav, f_vm, f_vp
    add.f32 f_vav, f_vbw, f_vav
    add.f32 f_vav, f_vup, f_vav
    mul.f32 f_vav, f_vav, 0.25

    mul.i64 r_idx, r_y, NX
    add.i64 r_idx, r_idx, r_x
    add.i64 r_idx, r_dst, r_idx
    sub.f32 f_a, f_up, f_um
    sub.f32 f_bv, f_vup, f_vbw
    add.f32 f_a, f_a, f_bv
    madd.f32 f_result, f_a, -0.1, f_hav
    st.f32 [h + r_idx], f_result
    sub.f32 f_a, f_fxp, f_fxm
    sub.f32 f_bv, f_fyp, f_fym
    add.f32 f_a, f_a, f_bv
    madd.f32 f_result, f_a, -0.1, f_uav
    st.f32 [hu + r_idx], f_result
    sub.f32 f_a, f_gxp, f_gxm
    sub.f32 f_bv, f_gyp, f_gym
    add.f32 f_a, f_a, f_bv
    madd.f32 f_result, f_a, -0.1, f_vav
    st.f32 [hv + r_idx], f_result
    loop r_x, 1, NX, update_x
    loop r_y, 1, NY, update_y

    ld.f32 f_hmin, [h + r_dst]
    mov.f32 f_hmax, f_hmin
    mov.i64 r_i, 0
find_range:
    add.i64 r_idx, r_dst, r_i
    ld.f32 f_height, [h + r_idx]
    cmp.lt.f32 c_lower, f_height, f_hmin
    sel.f32 f_hmin, c_lower, f_height, f_hmin
    cmp.gt.f32 c_higher, f_height, f_hmax
    sel.f32 f_hmax, c_higher, f_height, f_hmax
    loop r_i, 1, PLANE, find_range
    sub.f32 f_inv, f_hmax, f_hmin
    add.f32 f_inv, f_inv, 1e-9
    div.f32 f_inv, 255.0, f_inv

    mov.i64 r_y, 0
draw_y:
    udiv.i64 r_j, r_y, 3
    mul.i64 r_row, r_j, NX
    add.i64 r_row, r_dst, r_row
    mov.i64 r_x, 0
draw_x:
    udiv.i64 r_i, r_x, 3
    add.i64 r_idx, r_row, r_i
    ld.f32 f_height, [h + r_idx]
    shade r_brightness, f_height, f_hmin, f_inv
    or.i64 r_color, r_brightness, 0xff000000
    pixel r_x, r_y, r_color
    loop r_x, 1, 1536, draw_x
    loop r_y, 1, 768, draw_y
    flush
    loop r_step, 1, FRAMES, frame
    halt
