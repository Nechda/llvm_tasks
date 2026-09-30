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
endfunction()

# Tasks are added simply as subdirectories
add_subdirectory(task_1)
