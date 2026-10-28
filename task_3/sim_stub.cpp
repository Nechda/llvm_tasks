#include "sim.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace {
constexpr int Width = 1536;
constexpr int Height = 768;
uint64_t frames;
uint64_t pixels;
uint64_t checksum;
} // namespace

extern "C" void simInit(void) {
    frames = pixels = 0;
    checksum = 14695981039346656037ULL;
}

extern "C" void simPutPixel(int x, int y, int argb) {
    // Check both coordinates and traversal order, without allocating a window.
    const uint64_t position = pixels % (Width * Height);
    if (x != static_cast<int>(position % Width) ||
        y != static_cast<int>(position / Width) ||
        (static_cast<uint32_t>(argb) & 0xffffff00U) != 0xff000000U) {
        std::fprintf(stderr, "Invalid pixel: (%d, %d), color=%08x\n", x, y,
                     static_cast<unsigned>(argb));
        std::abort();
    }
    ++pixels;
    checksum = (checksum ^ static_cast<uint32_t>(argb)) * 1099511628211ULL;
}

extern "C" void simFlush(void) {
    ++frames;
    if (pixels != frames * Width * Height) {
        std::fprintf(stderr, "Incomplete frame\n");
        std::abort();
    }
    std::printf("[stub] frame=%llu pixels=%llu checksum=%016llx\n",
                static_cast<unsigned long long>(frames),
                static_cast<unsigned long long>(pixels),
                static_cast<unsigned long long>(checksum));
    std::fflush(stdout);
}

extern "C" void simExit(void) {
    std::printf("[stub] finished: frames=%llu pixels=%llu\n",
                static_cast<unsigned long long>(frames),
                static_cast<unsigned long long>(pixels));
}
