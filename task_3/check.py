#!/usr/bin/env python3
"""Compare native original IR with generated IR and LLVM Interpreter (3 frames)."""

import argparse
from pathlib import Path
import shlex
import subprocess
import tempfile


def run(*args):
    return subprocess.check_output(args, text=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--clang", required=True)
    parser.add_argument("--cxx", required=True)
    parser.add_argument("--cxx-flags", default="")
    parser.add_argument("--link-flags", default="")
    args = parser.parse_args()
    source = Path(__file__).resolve().parent
    build = args.build_dir.resolve()
    with tempfile.TemporaryDirectory(prefix="task3-check-") as directory:
        tmp = Path(directory)
        original = (source.parent / "task_1/app_Os.ir").read_text()
        # Keep the complete original app, changing only its execution duration.
        limit = "icmp eq i32 %207, 20000"
        if original.count(limit) != 1:
            raise RuntimeError("The frame limit in app_Os.ir has changed")
        (tmp / "reference.ll").write_text(original.replace(limit, "icmp eq i32 %207, 3"))
        (tmp / "driver.cpp").write_text(
            '#include "sim.h"\nextern "C" void app();\n'
            'int main() { simInit(); app(); simExit(); }\n'
        )
        run(str(build / "app_ir_gen"), "--frames", "3", "-o", str(tmp / "generated.ll"))
        results = {}
        for name in ("reference", "generated"):
            run(args.clang, "-c", "-O0", str(tmp / f"{name}.ll"),
                "-o", str(tmp / f"{name}.o"))
            run(args.cxx, *shlex.split(args.cxx_flags), "-std=c++17", "-I", str(source),
                str(tmp / "driver.cpp"), str(source / "sim_stub.cpp"),
                str(tmp / f"{name}.o"), *shlex.split(args.link_flags),
                "-o", str(tmp / name))
            results[name] = run(str(tmp / name))
        results["interpreter"] = run(str(build / "app_interpret"), "--frames", "3")
        for name, result in results.items():
            print(f"=== {name} ===\n{result}", end="")
        if len(set(results.values())) != 1:
            raise RuntimeError("Frame checksums differ")
        print("OK: original IR, generated IR and Interpreter agree on all 3 frames")


if __name__ == "__main__":
    main()
