#pragma once

// ABI-compatible with llvm_course/SDL/sim.h. Implemented by either sim_stub.cpp
// or the real SDL/sim.c, selected when configuring CMake.
#ifdef __cplusplus
extern "C" {
#endif
void simInit(void);
void simPutPixel(int x, int y, int argb);
void simFlush(void);
void simExit(void);
#ifdef __cplusplus
}
#endif
