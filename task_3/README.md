# Task 3 — генератор LLVM IR и исполнение app

`app_ir_gen.cpp` строит функцию `void app()` из `../task_1/app_Os.ir` через `IRBuilder`.

Номера `v[...]` и `bb[...]` соответствуют SSA-значениям и блокам исходного IR.
Сохранены векторные операции `<2 x float>` и интринсики  `fmuladd`, `smax`, `umin`, `memset`.

## Сборка

Задача собирается независимо от остальных задач и по умолчанию использует графические стабы — SDL и репозиторий `llvm_course` для этого не нужны.

`LLVM_DIR` указывает на CMake-конфигурацию установленного или собранного LLVM. Путь можно получить через `llvm-config --cmakedir`:

```bash
cmake -S task_3 -B build/task_3 \
  -DLLVM_DIR="$(llvm-config --cmakedir)" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/task_3 -j4
```

Команда `llvm-config` должна быть доступна в `PATH`. Если она имеет номер версии
в имени, используй соответствующее имя, например `llvm-config-22`.

В моем окружении была проверка с LLVM 24.0.0. Необходимы библиотеки: `LLVMCore`, `LLVMSupport`, `LLVMInterpreter`.

## Две программы

### 1. Генерация IR

```bash
build/task_3/app_ir_gen -o build/task_3/app.ll
# Без -o модуль печатается в stdout.
```

Строит IR и проверяет его через `verifyModule`.

### 2. Генерация и интерпретация

```bash
build/task_3/app_interpret --frames 1
# Дополнительно сохранить исходный сгенерированный модуль:
build/task_3/app_interpret --frames 1 -o build/task_3/app.ll
```

Используется `EngineBuilder(...).setEngineKind(EngineKind::Interpreter)`
и `ExecutionEngine::runFunction(app, {})`.
Без `--frames` будут исполнены все 20 000 кадров; для проверки удобнее выбрать например 1 фрейм.

Графические функции — внешние объявления в IR и настоящие функции с C ABI,
слинкованные с `app_interpret`. Адаптеры `GenericValue` регистрируются через
`DynamicLibrary::AddSymbol("lle_X_simPutPixel", ...)` и аналогично для
`simFlush`.

Адаптеры извлекают аргументы и вызывают слинкованные `simPutPixel`/`simFlush`.
До исполнения вызывается `simInit`, после — `simExit`.

Стабы в `sim_stub.cpp` проверяют порядок и координаты пикселей, диапазон цвета,
полноту кадров и печатают накопительную контрольную сумму. Например:

```text
[stub] frame=1 pixels=1179648 checksum=ecf6ae8acb2327e0
[stub] finished: frames=1 pixels=1179648
```

## Запуск с настоящей графикой

Вместо стабов можно слинковать `SDL/sim.c` из репозитория курса.

```bash
cmake -S task_3 -B build/task_3_sdl \
  -DLLVM_DIR="$(llvm-config --cmakedir)" \
  -DTASK3_USE_SDL=ON \
  -DLLVM_COURSE_ROOT=/path/to/llvm_course
cmake --build build/task_3_sdl -j4
build/task_3_sdl/app_interpret --frames 3
```

Используется только `sim.c`: роль `start.c` выполняет `main.cpp`.
Поведение окна при завершении определяется `simExit` из курса
(ожидание закрытия окна).

## CMake-цели и проверка результата

```bash
cmake --build build/task_3 --target task3_dump_ir # создать app.ll
cmake --build build/task_3 --target task3_run     # исполнить один кадр
cmake --build build/task_3 --target task3_check   # сравнить три кадра
```

`task3_check` доступна со стабами, Python 3 и Clang, способным прочитать
`task_1/app_Os.ir` (в данной среде используется Clang из build-tree LLVM).
`check.py` во временном каталоге ограничивает исходный IR тремя кадрами,
компилирует исходный и сгенерированный IR, запускает их с одинаковыми стабами
и сравнивает результат с исполнением через Interpreter.
Это проверяет численный результат, включая смену буферов и развитие волн,
а также количество, порядок и цвета всех пикселей через контрольные суммы.

Проверенный результат — совпадение всех трёх запусков:

| Кадр | Накопительное число пикселей | Контрольная сумма |
| --- | ---: | --- |
| 1 | 1 179 648 | `ecf6ae8acb2327e0` |
| 2 | 2 359 296 | `7c3c891ae0faf7f9` |
| 3 | 3 538 944 | `66bca24457862d80` |
