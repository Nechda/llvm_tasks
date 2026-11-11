#include "asm.h"
#include "ensure.h"
#include "simulator.h"
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    try {
        ENSURE_MSG(argc == 2 || argc == 4,
                   "usage: asm_sim input.asm [--frames N]");
        unsigned frames = 0;
        if (argc == 4) {
            ENSURE_MSG(std::string(argv[2]) == "--frames",
                       "expected --frames N");
            std::string value = argv[3];
            size_t end;
            auto n = std::stoull(value, &end);
            ENSURE_MSG(end == value.size() && n != 0 && n <= 0x7fffffff,
                       "--frames must be in [1, 2147483647]");
            frames = static_cast<unsigned>(n);
        }
        std::ifstream input(argv[1]);
        ENSURE_MSG(input, "cannot open input assembly");
        task4::simulate(task4::parse(input, frames));
    } catch (const std::exception &e) {
        std::cerr << "asm_sim: " << e.what() << '\n';
        return 1;
    }
}
