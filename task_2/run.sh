#!/usr/bin/env bash
# Полный пайплайн трассировки использования IR инструкций графического
# приложения (task_1/app.c) через инструментирующий pass TracePass.
#
# Шаги:
#   1) dump IR (-O2 или -Os);
#   2) инструментирование pass'ом (opt -load-pass-plugin);
#   3) компиляция инструментированного IR + trace_rt.c в объектные файлы
#      (--target=aarch64-apple-darwin / x86_64-apple-darwin);
#   4) [на маке] линковка с sim.c/start.c + SDL2 и запуск (трасса в bin/);
#   5) анализ паттернов (окна 1..5) по трассе.
#
# Использование:
#   ./run.sh stage O2|Os [target]   # сделать всё, что можно на этом хосте
#   ./run.sh mac   O2|Os           # подсказка команд для сборки/запуска на маке
#   ./run.sh analyze O2|Os         # анализ трассы (после запуска)
#
# Переменные окружения:
#   LLVM_BUILD     -- build-директория LLVM (clang, opt, llvm-config),
#                     по умолчанию ~/github/llvm-project/build
#   TRACE_DIR      -- рабочая директория (по умолчанию ./trace_build)
#   TRACE_MAX      -- лимит пар для run.sh mac (LLVM_TRACE_MAX)
#   TOP            -- корень репозитория llvm_tasks (по умолчанию ..)

set -euo pipefail

OPT="${2:-O2}"
case "$OPT" in O2|Os) ;; *) echo "bad opt level: $OPT" >&2; exit 1;; esac

LLVM_BUILD="${LLVM_BUILD:-$HOME/github/llvm-project/build}"
TOP="${TOP:-$(cd "$(dirname "$0")/.." && pwd)}"
SRC_APP="$TOP/task_1/app.c"
SDL_DIR="${LLVM_COURSE_ROOT:-$HOME/github/llvm_course}/SDL"
OUT="${TRACE_DIR:-$(dirname "$0")/trace_build}"
mkdir -p "$OUT"

CLANG="$LLVM_BUILD/bin/clang"
OPT_BIN="$LLVM_BUILD/bin/opt"
PLUGIN="$OUT/TracePass.so"

case "${1:-}" in
stage)
    TARGET="${3:-aarch64-apple-darwin}"
    echo "==> [1/4] dump IR ($OPT, target $TARGET)"
    "$CLANG" -S -emit-llvm "-$OPT" --target="$TARGET" \
        -I "$SDL_DIR" "$SRC_APP" -o "$OUT/app_${OPT}.ll"
    "$OPT_BIN" -passes=verify -disable-output "$OUT/app_${OPT}.ll"

    echo "==> [2/4] build pass plugin"
    # У некоторых clang нет bundled C++ stdlib; тогда задайте
    # LLVM_TRACE_SYSROOT=<sysroot> (например ~/.../os_sdk на dev-хостах).
    SYSROOT_ARGS=""
    if [ -n "${LLVM_TRACE_SYSROOT:-}" ]; then
        SYSROOT_ARGS="--sysroot=${LLVM_TRACE_SYSROOT}"
    fi
    CXXFLAGS="$($LLVM_BUILD/bin/llvm-config --cxxflags)"
    # shellcheck disable=SC2086
    "$CLANG++" -shared -fPIC -O2 $SYSROOT_ARGS $CXXFLAGS \
        "$TOP/task_2/TracePass.cpp" -o "$PLUGIN"

    echo "==> [3/4] instrument"
    LLVM_TRACE_MAP="$OUT/trace_map_${OPT}.txt" \
        "$OPT_BIN" -load-pass-plugin="$PLUGIN" -passes=trace-instr \
        "$OUT/app_${OPT}.ll" -S -o "$OUT/app_${OPT}_instr.ll"
    "$OPT_BIN" -passes=verify -disable-output "$OUT/app_${OPT}_instr.ll"

    echo "==> [4/4] cross-compile instrumented app ($TARGET)"
    CROSS_SYSROOT_ARGS=""
    if [ -n "${LLVM_TRACE_SYSROOT:-}" ]; then
        # только для *-linux-* целей, на mac не нужно
        case "$TARGET" in
            *linux*) CROSS_SYSROOT_ARGS="--sysroot=${LLVM_TRACE_SYSROOT}" ;;
        esac
    fi
    # shellcheck disable=SC2086
    "$CLANG" "-$OPT" --target="$TARGET" $CROSS_SYSROOT_ARGS \
        -c "$OUT/app_${OPT}_instr.ll" -o "$OUT/app_${OPT}_instr.o"
    # shellcheck disable=SC2086
    "$CLANG" -O2 --target="$TARGET" $CROSS_SYSROOT_ARGS \
        -c "$TOP/task_2/trace_rt.c" -o "$OUT/trace_rt.o" 2>/dev/null || \
        echo "note: trace_rt.o не собрался на этом хосте (нет SDK для $TARGET) -- соберите его на маке (см. ./run.sh mac)"
    echo "готово: $OUT/app_${OPT}_instr.o, $OUT/trace_rt.o, $OUT/trace_map_${OPT}.txt"
    ;;

mac)
    TARGET="${3:-aarch64-apple-darwin}"
    cat <<EOF
==> На маке (линковка и запуск; используется нативный clang / Apple clang):

  # если trace_rt.o ещё не собран:
  clang -O2 -c $TOP/task_2/trace_rt.c -o $OUT/trace_rt.o

  # сборка sim.c/start.c и линковка (SDL2 из brew):
  clang -O2 -I $SDL_DIR -I /opt/homebrew/include/SDL2 \\
      -c $SDL_DIR/sim.c -o sim.o
  clang -O2 -I $SDL_DIR -c $SDL_DIR/start.c -o start.o
  clang $OUT/app_${OPT}_instr.o $OUT/trace_rt.o sim.o start.o \\
      -L /opt/homebrew/lib -lSDL2 -lm -o app_${OPT}_instr

  # запуск (трасса: $OUT/trace_${OPT}.bin; каждая пара -- 8 байт,
  # полный прогон даёт миллиарды пар, поэтому имеет смысл собрать по квоте):
  LLVM_TRACE_FILE=$OUT/trace_${OPT}.bin \\
  LLVM_TRACE_MAP=$OUT/trace_map_${OPT}.txt \\
  LLVM_TRACE_MAX=${TRACE_MAX:-0} LLVM_TRACE_EXIT=1 ./app_${OPT}_instr

Затем на любом хосте:
  ./run.sh analyze $OPT
EOF
    ;;

analyze)
    echo "==> анализ паттернов (окна 1..5)"
    if [ ! -x "$OUT/analyze_bin" ] || [ "$TOP/task_2/analyze.cpp" -nt \
         "$OUT/analyze_bin" ]; then
        if [ -x "$LLVM_BUILD/bin/clang++" ]; then
            CXX="$LLVM_BUILD/bin/clang++"
        else
            CXX=clang++
        fi
        SYSROOT_ARGS=""
        [ -n "${LLVM_TRACE_SYSROOT:-}" ] && \
            SYSROOT_ARGS="--sysroot=${LLVM_TRACE_SYSROOT}"
        # shellcheck disable=SC2086
        $CXX $SYSROOT_ARGS -O2 -std=c++17 "$TOP/task_2/analyze.cpp" \
            -o "$OUT/analyze_bin"
    fi
    "$OUT/analyze_bin" "$OUT/trace_${OPT}.bin" "$OUT/trace_map_${OPT}.txt" \
        "$OUT/patterns_${OPT}.txt" --top 30 \
        && cp "$OUT/patterns_${OPT}.txt" "$TOP/task_2/stats/patterns_${OPT}.txt"
    echo "stats: $TOP/task_2/stats/patterns_${OPT}.txt"
    ;;

*)
    echo "usage: $0 stage|mac|analyze O2|Os [target]" >&2; exit 1;;
esac
