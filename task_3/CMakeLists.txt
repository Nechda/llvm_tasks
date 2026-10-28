cmake_minimum_required(VERSION 3.16)

# Also supports an independent build: cmake -S task_3 -B build/task_3 ...
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(task_3 LANGUAGES C CXX)
else()
  enable_language(CXX)
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "task_3: using LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")

add_library(task3_ir STATIC app_ir_gen.cpp)
target_compile_features(task3_ir PUBLIC cxx_std_17)
target_include_directories(task3_ir SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITION_FLAGS NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_options(task3_ir PUBLIC ${LLVM_DEFINITION_FLAGS})
if(NOT LLVM_ENABLE_RTTI)
  if(MSVC)
    target_compile_options(task3_ir PUBLIC /GR-)
  else()
    target_compile_options(task3_ir PUBLIC -fno-rtti)
  endif()
endif()
llvm_map_components_to_libnames(TASK3_CORE_LIBS Core Support)
target_link_libraries(task3_ir PUBLIC ${TASK3_CORE_LIBS})

add_executable(app_ir_gen main.cpp)
target_link_libraries(app_ir_gen PRIVATE task3_ir)

add_executable(app_interpret main.cpp)
target_compile_definitions(app_interpret PRIVATE TASK3_EXECUTE=1)
llvm_map_components_to_libnames(TASK3_ENGINE_LIBS Interpreter ExecutionEngine)
target_link_libraries(app_interpret PRIVATE task3_ir ${TASK3_ENGINE_LIBS})

option(TASK3_USE_SDL "Link app_interpret with real llvm_course SDL functions" OFF)
if(TASK3_USE_SDL)
  set(LLVM_COURSE_ROOT "${LLVM_COURSE_ROOT}" CACHE PATH "Path to llvm_course")
  if(NOT EXISTS "${LLVM_COURSE_ROOT}/SDL/sim.c")
    message(FATAL_ERROR "TASK3_USE_SDL requires LLVM_COURSE_ROOT with SDL/sim.c")
  endif()
  find_package(SDL2 REQUIRED)
  target_sources(app_interpret PRIVATE "${LLVM_COURSE_ROOT}/SDL/sim.c")
  target_include_directories(app_interpret PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(app_interpret PRIVATE ${SDL2_LIBRARIES})
else()
  target_sources(app_interpret PRIVATE sim_stub.cpp)
endif()

add_custom_target(task3_dump_ir
  COMMAND $<TARGET_FILE:app_ir_gen> -o "${CMAKE_CURRENT_BINARY_DIR}/app.ll"
  DEPENDS app_ir_gen
  COMMENT "Building app.ll using LLVM IRBuilder"
  VERBATIM)
add_custom_target(task3_run
  COMMAND $<TARGET_FILE:app_interpret> --frames 1
  DEPENDS app_interpret
  USES_TERMINAL
  COMMENT "Interpreting one frame of app"
  VERBATIM)

if(NOT TASK3_USE_SDL)
  find_package(Python3 QUIET COMPONENTS Interpreter)
  find_program(TASK3_CLANG NAMES clang HINTS "${LLVM_TOOLS_BINARY_DIR}")
  if(Python3_Interpreter_FOUND AND TASK3_CLANG)
    add_custom_target(task3_check
      COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/check.py"
        --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
        --clang "${TASK3_CLANG}" --cxx "${CMAKE_CXX_COMPILER}"
        "--cxx-flags=${CMAKE_CXX_FLAGS}"
        "--link-flags=${CMAKE_EXE_LINKER_FLAGS}"
      DEPENDS app_ir_gen app_interpret
      USES_TERMINAL
      COMMENT "Comparing original IR, generated IR and interpreted frames"
      VERBATIM)
  endif()
endif()
