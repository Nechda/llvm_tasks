# llvm_tasks

Сборка: `cmake -B build -DLLVM_COURSE_ROOT=<путь до репозитория llvm_course> && cmake --build build` (например `-DLLVM_COURSE_ROOT=~/github/llvm_course`).

`LLVM_COURSE_ROOT` обязателен — оттуда берутся `sim.h`, `sim.c`, `start.c`.

Опционально `-DSDL2_DIR=<путь до SDL2>` для нестандартной установки; если SDL2 не найден, таски только компилируются без линковки.

Новая задача: подключить её директорию через `add_subdirectory` и вызвать `add_llvm_task(<имя> app.c)`.
