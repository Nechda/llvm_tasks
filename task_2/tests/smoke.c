// Дымовой тест пайплайна трассировки (без SDL).
// Аргументы зависят от argc, чтобы код не сворачивался в константы.
#include <stdio.h>

int g = 0;

__attribute__((noinline)) int work(int a, int b) {
    int s = 0;
    for (int i = 0; i < 100; ++i) {
        int m = a * b + i;
        s += m ^ (i << 1);
        if (s > 1000)
            s -= 7;
    }
    volatile int t = s;
    return (int)t + g;
}

int main(int argc, char **argv) {
    g = 0;
    work(argc + 2, argc + 3);
    printf("%d\n", work(argc + 2, argc + 3));
    return g == 0;
}
