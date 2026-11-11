#!/usr/bin/env python3
"""Compare three complete frames against the original LLVM IR."""
import argparse
from pathlib import Path
import subprocess
import tempfile
import shlex


def run(*args):
    return subprocess.check_output([str(a) for a in args], text=True, timeout=180)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--sim", type=Path, required=True)
    parser.add_argument("--clang", required=True)
    parser.add_argument("--cxx", required=True)
    parser.add_argument("--cxx-flags", default="")
    parser.add_argument("--link-flags", default="")
    args = parser.parse_args()
    source = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix="task4-check-") as directory:
        tmp = Path(directory)
        reference = (source.parent / "task_1/app_Os.ir").read_text()
        limit = "icmp eq i32 %207, 20000"
        assert reference.count(limit) == 1
        (tmp / "reference.ll").write_text(reference.replace(limit, "icmp eq i32 %207, 3"))
        (tmp / "driver.cpp").write_text(
            '#include "sim.h"\nextern "C" void app();\n'
            'int main() { simInit(); app(); simExit(); }\n'
        )
        run(args.clang, "-O0", "-c", tmp / "reference.ll", "-o", tmp / "reference.o")
        run(args.cxx, *shlex.split(args.cxx_flags), "-std=c++17", "-I", source.parent / "task_3",
            tmp / "driver.cpp", source.parent / "task_3/sim_stub.cpp",
            tmp / "reference.o", *shlex.split(args.link_flags), "-o", tmp / "reference")
        expected = run(tmp / "reference")
        print("reference:\n" + expected, end="")
        for mode in ("--direct", "--calls"):
            for optimized in (False, True):
                flags = ["--opt"] if optimized else []
                name = mode[2:] + ("-opt" if optimized else "")
                actual = run(args.exe, source / "app.asm", tmp / f"{name}.ll",
                             mode, *flags, "--run", "--frames", "3")
                if actual != expected:
                    raise RuntimeError(f"{name}: frame mismatch\n{actual}")
                print(f"OK: {name} agrees with original IR on all three frames")
        actual = run(args.sim, source / "app.asm", "--frames", "3")
        if actual != expected:
            raise RuntimeError(f"simulator: frame mismatch\n{actual}")
        print("OK: simulator agrees with original IR on all three frames")


if __name__ == "__main__":
    main()
