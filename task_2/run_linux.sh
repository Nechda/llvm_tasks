#!/usr/bin/env bash
# Один скрипт — весь пайплайн трассировки на linux-хосте (без SDL):
#   1) IR (-O2/-Os) + инструментирование pass'ом TracePass;
#   2) линковка с tests/sim_stub.c (стаб симулятора);
#   3) запуск с квотой LLVM_TRACE_MAX и таймаутом RUN_TIMEOUT секунд;
#   4) анализ паттернов (окна 1..5) -> trace_build/patterns_*.txt + stats/.
#
# Использование: ./run_linux.sh [O2] [Os] ...   (по умолчанию: оба)
# Env:
#   LLVM_BUILD       -- build LLVM            (default ~/github/llvm-project/build)
#   LLVM_COURSE_ROOT -- репозиторий llvm_course (default ~/tmp/llvm_course)
#   TOP              -- корень llvm_tasks       (default: родитель этого скрипта)
#   TRACE_MAX        -- квота пар               (default 100000000)
#   RUN_TIMEOUT      -- таймаут запуска, сек    (default 60)
#   SYSROOT          -- sysroot для clang       (default: авто, $HOME/.os_sdk,
#                       отключить: SYSROOT=none)

set -euo pipefail

if [ $# -eq 0 ]; then set -- O2 Os; fi
OPTS=("$@")

LLVM_BUILD="${LLVM_BUILD:-$HOME/github/llvm-project/build}"
export LLVM_COURSE_ROOT="${LLVM_COURSE_ROOT:-$HOME/tmp/llvm_course}"
TOP="${TOP:-$(cd "$(dirname "$0")/.." && pwd)}"
TRACE_MAX="${TRACE_MAX:-100000000}"
RUN_TIMEOUT="${RUN_TIMEOUT:-60}"
OUT="$TOP/task_2/trace_build"
TARGET="x86_64-unknown-linux-gnu"

CLANG="$LLVM_BUILD/bin/clang"
OPTBIN="$LLVM_BUILD/bin/opt"
CLANGXX="$LLVM_BUILD/bin/clang++"

if [ "${SYSROOT:-auto}" = "auto" ] && [ -d "$HOME/.os_sdk/usr/include" ]; then
    SYSROOT="$HOME/.os_sdk"
else
    SYSROOT="${SYSROOT%/}"            # 'none' -> пустая строка
    [ "$SYSROOT" = "none" ] && SYSROOT=""
fi
SR_ARGS=()
[ -n "$SYSROOT" ] && SR_ARGS=(--sysroot="$SYSROOT")

export LLVM_TRACE_SYSROOT="$SYSROOT"

for OPT in "${OPTS[@]}"; do
    case "$OPT" in O2|Os) ;; *) echo "bad opt level: $OPT" >&2; exit 1;; esac

    echo "===== [$OPT] 1/4: IR + instrumentation ====="
    "$CLANG" "${SR_ARGS[@]}" -S -emit-llvm "-$OPT" --target="$TARGET" \
        -I "$LLVM_COURSE_ROOT/SDL" "$TOP/task_1/app.c" -o "$OUT/app_${OPT}.ll"

    echo "===== [$OPT] 2/4: build pass plugin ====="
    "$CLANGXX" "${SR_ARGS[@]}" -shared -fPIC -O2 \
        $($LLVM_BUILD/bin/llvm-config --cxxflags) \
        "$TOP/task_2/TracePass.cpp" -o "$OUT/TracePass.so"

    echo "===== [$OPT] 3/4: instrument ====="
    LLVM_TRACE_MAP="$OUT/trace_map_${OPT}.txt" \
        "$OPTBIN" -load-pass-plugin="$OUT/TracePass.so" -passes=trace-instr \
        "$OUT/app_${OPT}.ll" -S -o "$OUT/app_${OPT}_instr.ll"
    "$OPTBIN" -passes=verify -disable-output "$OUT/app_${OPT}_instr.ll"

    echo "===== [$OPT] 4a: link (sim_stub, без SDL) ====="
    "$CLANG" "${SR_ARGS[@]}" -O2 "$OUT/app_${OPT}_instr.ll" \
        "$TOP/task_2/trace_rt.c" \
        -I "$LLVM_COURSE_ROOT/SDL" "$TOP/task_2/tests/sim_stub.c" \
        "$LLVM_COURSE_ROOT/SDL/start.c" -o "$OUT/app_${OPT}_linux_bin"

    echo "===== [$OPT] 4b: run (timeout ${RUN_TIMEOUT}s, quota $TRACE_MAX) ====="
    rm -f "$OUT/trace_${OPT}.bin"
    set +e
    timeout "$RUN_TIMEOUT" env \
        LLVM_TRACE_FILE="$OUT/trace_${OPT}.bin" \
        LLVM_TRACE_MAX="$TRACE_MAX" LLVM_TRACE_EXIT=1 \
        "$OUT/app_${OPT}_linux_bin"
    rc=$?
    set -e
    if [ $rc -ne 0 ] && [ $rc -ne 124 ]; then
        echo "run failed: exit $rc" >&2; exit $rc
    fi
    # 124 -- timeout: трасса может быть неполной, но анализ всё равно осмыслен.
    ls -la "$OUT/trace_${OPT}.bin"

    echo "===== [$OPT] analyze ====="
    "$CLANGXX" "${SR_ARGS[@]}" -O2 -std=c++17 "$TOP/task_2/analyze.cpp" \
        -o "$OUT/analyze_bin"
    "$OUT/analyze_bin" "$OUT/trace_${OPT}.bin" "$OUT/trace_map_${OPT}.txt" \
        "$OUT/patterns_${OPT}.txt" --top 30
    cp "$OUT/patterns_${OPT}.txt" "$TOP/task_2/stats/patterns_${OPT}.txt"
    cp "$OUT/trace_map_${OPT}.txt" "$TOP/task_2/stats/trace_map_${OPT}.txt"
    echo "===== [$OPT] done: stats/patterns_${OPT}.txt ====="
done
