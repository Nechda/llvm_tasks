cmake_minimum_required(VERSION 3.16)
project(llvm_tasks C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Path to the llvm_course repository root (contains SDL/sim.h, SDL/sim.c, SDL/start.c)
# MUST be passed via: cmake -B build -DLLVM_COURSE_ROOT=~/github/llvm_course
set(LLVM_COURSE_ROOT "" CACHE PATH "Path to llvm_course repository root")

if(LLVM_COURSE_ROOT STREQUAL "")
  message(FATAL_ERROR
    "LLVM_COURSE_ROOT is not set. Pass the path to the llvm_course repository: "
    "cmake -DLLVM_COURSE_ROOT=~/github/llvm_course ...")
endif()

if(NOT EXISTS "${LLVM_COURSE_ROOT}/SDL/sim.h")
  message(FATAL_ERROR
    "llvm_course repository not found at '${LLVM_COURSE_ROOT}' "
    "(SDL/sim.h is missing). Check -DLLVM_COURSE_ROOT.")
endif()

set(LLVM_COURSE_SDL_DIR "${LLVM_COURSE_ROOT}/SDL")

# Locate SDL2 (headers and library) in standard paths
find_path(SDL2_INCLUDE_DIR NAMES SDL2/SDL.h
  PATHS ${SDL2_DIR}/include /usr/local/include/SDL2 /usr/include/SDL2)
find_library(SDL2_LIBRARY NAMES SDL2
  PATHS ${SDL2_DIR}/lib /usr/local/lib)

if(NOT SDL2_INCLUDE_DIR OR NOT SDL2_LIBRARY)
  message(WARNING
    "SDL2 not found: full execables are disabled, tasks are compile-checked only. "
    "Install libsdl2-dev or pass -DSDL2_DIR=/path/to/sdl2 to enable linking.")
endif()

# Adds a task target: sim.c and start.c from llvm_course are linked with <src>.
# If SDL2 is missing, only a compile check of <src> is performed.
# Additional targets <name>_ir_dump_O2 / <name>_ir_dump_Os generate LLVM IR
# from <src>: clang -S -emit-llvm -O2/-Os <in> -o <name>_O2.ir / <name>_Os.ir
function(add_llvm_task name src)
  if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
    add_executable(${name}
      ${LLVM_COURSE_SDL_DIR}/sim.c
      ${LLVM_COURSE_SDL_DIR}/start.c
      ${src})
    target_include_directories(${name} PRIVATE ${LLVM_COURSE_SDL_DIR} ${SDL2_INCLUDE_DIR})
    target_link_libraries(${name} PRIVATE ${SDL2_LIBRARY})
  else()
    add_library(${name} OBJECT ${src})
    target_include_directories(${name} PRIVATE ${LLVM_COURSE_SDL_DIR})
  endif()

  find_program(CLANG_EXE clang)
  if(CLANG_EXE)
    get_filename_component(base ${src} NAME_WE)
    set(ir_out_dir ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(OUTPUT ${ir_out_dir}/${base}_O2.ir
      COMMAND ${CLANG_EXE} -S -emit-llvm -O2 -I${LLVM_COURSE_SDL_DIR}
              ${CMAKE_CURRENT_SOURCE_DIR}/${src}
              -o ${ir_out_dir}/${base}_O2.ir
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
      COMMENT "Generating ${base}_O2.ir"
      VERBATIM)
    add_custom_command(OUTPUT ${ir_out_dir}/${base}_Os.ir
      COMMAND ${CLANG_EXE} -S -emit-llvm -Os -I${LLVM_COURSE_SDL_DIR}
              ${CMAKE_CURRENT_SOURCE_DIR}/${src}
              -o ${ir_out_dir}/${base}_Os.ir
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
      COMMENT "Generating ${base}_Os.ir"
      VERBATIM)
    add_custom_target(${base}_ir_O2 DEPENDS ${ir_out_dir}/${base}_O2.ir)
    add_custom_target(${base}_ir_Os DEPENDS ${ir_out_dir}/${base}_Os.ir)
  else()
    message(WARNING "clang not found, IR dump targets are not available")
  endif()
endfunction()

# Tasks are added simply as subdirectories
add_subdirectory(task_1)

# `make format`: run clang-format -i on all C/C++ sources in the repo
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB_RECURSE FORMAT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
  list(FILTER FORMAT_SOURCES EXCLUDE REGEX "/build/")
  add_custom_target(format
    COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running clang-format on ${FORMAT_SOURCES}"
    VERBATIM)
else()
  message(WARNING "clang-format not found, 'format' target is not available")
endif()
