# llvm_tasks

Сборка: `cmake -B build -DLLVM_COURSE_ROOT=<путь до репозитория llvm_course> && cmake --build build` (например `-DLLVM_COURSE_ROOT=~/github/llvm_course`).

`LLVM_COURSE_ROOT` обязателен — оттуда берутся `sim.h`, `sim.c`, `start.c`.

Опционально `-DSDL2_DIR=<путь до SDL2>` для нестандартной установки; если SDL2 не найден, таски только компилируются без линковки.

Новая задача: подключить её директорию через `add_subdirectory` и вызвать `add_llvm_task(<имя> app.c)`.

## Task 3: генерация IR через LLVM API и LLVM Interpreter

Самостоятельная сборка двух программ (по умолчанию с графическими стабами):

```bash
cmake -S task_3 -B build/task_3 \
  -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build build/task_3 -j4
build/task_3/app_ir_gen -o build/task_3/app.ll
build/task_3/app_interpret --frames 1
```

Подробности, подключение SDL и сравнение с исходным IR: [task_3/README.md](task_3/README.md).
