/* Стаб симулятора для host-сборки трейса без SDL (только для тестов).
 * На маке вместо него линкуется настоящий sim.c из llvm_course/SDL. */
#include "sim.h"

void simInit(void) {}
void simExit(void) {}
void simFlush(void) {}
void simPutPixel(int x, int y, int argb) {
    (void)x;
    (void)y;
    (void)argb;
}
int simRand(void) { return 0; }
int simHasClick(void) { return 0; }
int simGetClick(void) { return 0; }
