# Сборка плагина TracePass. Нужен собранный LLVM (clang, opt):
#   cmake -B build -DLLVM_TRACE_LLVM_DIR=/path/to/llvm-project/build
# Плагин линкуется против build-tree того же LLVM, которым делается
# инструментирование и кросс-компиляция (см. run.sh).

set(LLVM_TRACE_LLVM_DIR "" CACHE PATH "LLVM build dir (contains bin/opt)")

if(LLVM_TRACE_LLVM_DIR STREQUAL "")
  message(STATUS
    "task_2: LLVM_TRACE_LLVM_DIR is not set -> TracePass plugin is disabled")
  return()
endif()

if(NOT EXISTS "${LLVM_TRACE_LLVM_DIR}/bin/opt")
  message(FATAL_ERROR "task_2: ${LLVM_TRACE_LLVM_DIR}/bin/opt not found")
endif()

enable_language(CXX)
set(LLVM_DIR "${LLVM_TRACE_LLVM_DIR}/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG NO_DEFAULT_PATH)

message(STATUS "task_2: building TracePass against LLVM ${LLVM_PACKAGE_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_llvm_pass_plugin(TracePass TracePass.cpp)
