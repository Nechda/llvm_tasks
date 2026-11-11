#!/usr/bin/env python3
"""Exercise ISA boundaries and parser diagnostics in all execution modes."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True)
    parser.add_argument("--sim", required=True)
    args = parser.parse_args()
    program = [".buffer data, f32, 4"]
    checks = 0

    def check(code, register, expected):
        nonlocal checks
        checks += 1
        program.extend(code.strip().splitlines())
        program.extend([f"cmp.eq.i64 c_ok, {register}, {expected}",
                        f"br c_ok, check_{checks}, failure", f"check_{checks}:"])

    check("mov.i64 r_a, 0xffffffffffffffff\nadd.i64 r_a, r_a, 1", "r_a", 0)
    check("sub.i64 r_a, 0, 1", "r_a", "0xffffffffffffffff")
    check("mul.i64 r_a, r_a, 2", "r_a", -2)
    check("udiv.i64 r_a, 0xffffffffffffffff, 2", "r_a", "0x7fffffffffffffff")
    check("and.i64 r_a, r_a, 255\nxor.i64 r_a, r_a, 255\nor.i64 r_a, r_a, 8", "r_a", 8)
    check("cmp.lt.i64 c_negative, -1, 0\nsel.i64 r_a, c_negative, -12, 20", "r_a", -12)
    check("sel.i64 r_a, 0, -1, 17", "r_a", 17)
    check("itof.f32 f_a, -12\nadd.f32 f_a, f_a, 0.5\nftoi.i64 r_a, f_a", "r_a", -11)
    check("mov.f32 f_a, 9.0\nsub.f32 f_a, f_a, 1.0\nmul.f32 f_a, f_a, 3.0\n"
          "div.f32 f_a, f_a, 2.0\nftoi.i64 r_a, f_a", "r_a", 12)
    check("cmp.lt.f32 c_lt, -1.0, 0.0\nsel.f32 f_a, c_lt, 7.0, 9.0\n"
          "cmp.gt.f32 c_gt, f_a, 6.0\nsel.f32 f_a, c_gt, f_a, 0.0\n"
          "sel.f32 f_a, 1, f_a, 0.0\nftoi.i64 r_a, f_a", "r_a", 7)
    # Fused arithmetic would yield 1.0, while two roundings must yield 0.0.
    check("madd.f32 f_a, 4097.0, 4097.0, -16785408.0\nftoi.i64 r_a, f_a", "r_a", 0)
    check("shade r_a, -2.0, 0.0, 1.0", "r_a", 0)
    check("shade r_a, 300.0, 0.0, 1.0", "r_a", 255)
    check("shade r_a, 1.75, 0.5, 10.0", "r_a", 12)
    check("fill.f32 data, 0, 4, 7.0\nfill.f32 data, 4, 0, 99.0\n"
          "mov.i64 r_idx, 3\nst.f32 [data + r_idx], 9.0\n"
          "ld.f32 f_a, [data + 3]\nftoi.i64 r_a, f_a", "r_a", 9)
    check("ld.f32 f_a, [data]\nftoi.i64 r_a, f_a", "r_a", 7)
    check("mov.i64 r_a, 1\nalias_loop:\nloop r_a, r_a, 3, alias_loop", "r_a", 4)
    program.extend(["jmp success", "unreachable:", "jmp failure", "success:",
                    "br 1, done, failure", "done:", "halt", "failure:",
                    "pixel -1, 0, 0", "halt"])
    with tempfile.TemporaryDirectory(prefix="task4-isa-") as directory:
        source = Path(directory) / "test.asm"
        source.write_text("\n".join(program) + "\n")
        modes = ([args.exe, str(source), "--direct", "--run"],
                 [args.exe, str(source), "--calls", "--run"],
                 [args.sim, str(source)])
        for command in modes:
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            assert result.returncode == 0, result.stderr
            assert result.stdout == "[stub] finished: frames=0 pixels=0\n", result.stdout
        invalid = [
            ("jmp missing\n", "undefined label"),
            ("mov.f32 r_bad, 1.0\nhalt\n", "wrong register type"),
            ("again:\nhalt\nagain:\nhalt\n", "duplicate label"),
            ("call foo\nhalt\n", "unknown instruction"),
            ("mov.i64 r_a, 1\n", "falls off the end"),
            (".buffer data, f32, 4\nld.f32 f_a, [data + f_index]\nhalt\n", "wrong register type"),
            ("mov.i64 r_a\nhalt\n", "wrong operand count"),
        ]
        for text, diagnostic in invalid:
            source.write_text(text)
            for command in ([args.exe, str(source), str(Path(directory) / "out.ll"), "--cfg"],
                            [args.sim, str(source)]):
                result = subprocess.run(command, capture_output=True, text=True, timeout=30)
                assert result.returncode != 0 and diagnostic in result.stderr, result.stderr
                assert "ENSURE(" in result.stderr and ".cpp:" in result.stderr, result.stderr
    print(f"OK: {checks} semantic checks in all three modes; parser diagnostics checked")


if __name__ == "__main__":
    main()
