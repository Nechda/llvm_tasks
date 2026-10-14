# task_2 — трасса использования IR инструкций (User <- Operand)

Instrumentирующий LLVM pass + рантайм трейсинга + анализ часто повторяемых
паттернов исполнения для приложения `task_1/app.c` на -O2 и -Os.

## Состав

- `TracePass.cpp` — модульный pass (New PM, plugin для `opt`, имя
  `trace-instr`). Каждой инструкции модуля присваивается уникальный uint32 id
  (соответствие выгружается в map-файл). Для каждой инструкции-User (PHI
  пропускается) перед ней вставляются вызовы
  `void __trace_pair(i32 userId, i32 operandId)` — по одному на каждый
  операнд; `operandId` — id инструкции-операнда либо 0, если операнд не
  инструкция (константа/аргумент/глобал).
- `trace_rt.c` — рантайм: `__trace_pair` пишет пары
  `(uint32 user, uint32 operand)` в бинарный файл трассы
  (`"TRC1"`, uint64 num_pairs, затем пары).
- `analyze.cpp` — анализ (C++, компилируется на лету через `$CXX`):
  частоты opcodes User'ов, частоты пар `User <- Operand` и n-граммы (окна) длины
  1..5 по последовательности инструкций и пар; вывод — `count<TAB>доля<TAB>ключ`.
  Для той же задачи есть удалённая python-версия; C++ обрабатывает 100M пар
  за ~8 секунд.
- `run.sh` — пайплайн (см. usage) для -O2 и -Os, с кросс-компиляцией
  под mac (`aarch64-apple-darwin` по умолчанию).
- `tests/smoke.c` — дымовой тест пайплайна без SDL.
- `stats/` — собранная статистика (`patterns_O2.txt`, `patterns_Os.txt`).

## Как воспроизвести

0. Собрать LLVM (clang, opt, lld) из `~/github/llvm-project`, например:
   `cmake -G Ninja -B build -S llvm -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD="X86;AArch64" && ninja -C build clang opt lld`

1. `./run.sh stage O2` и `./run.sh stage Os` — IR, инструментирование и
   объектные файлы (`trace_build/app_${OPT}_instr.o`, `trace_rt.o`,
   `trace_map_${OPT}.txt`).

2. На маке (`./run.sh mac O2` печатает команды): слинковать с `sim.c`,
   `start.c`, SDL2, запустить; рантайм окружение:
   `LLVM_TRACE_FILE`, `LLVM_TRACE_MAP`, `LLVM_TRACE_MAX` (лимит пар),
   `LLVM_TRACE_EXIT=1` — досрочный `_exit(0)` по достижении лимита
   (полный прогон на 20000 шагов даёт десятки миллиардов пар, поэтому
   трасса собирается по квоте, пока покрывает устойчивые паттерны гейта).

3. `./run.sh analyze O2` / ` Os` — статистика кладётся в `stats/`.

## Замечания по реализации

- PHI-узлы не инструментируются как User (по условию), но встречаются в
  трассе как Operand.
- Несколько последовательных пар от одного и того же User’а схлопываются в
  n-граммном анализе в одно вхождение (окно — по уникальным инструкциям).
- Пасс запускается поверх уже оптимизированного IR: трасса отражает
  финальный IR (как используются инструкции оптимизированного кода).

## Статистика в этом репозитории

`stats/patterns_{O2,Os}.txt` (top-30, полные: частоты opcode, пар, n-граммы
окон 1..5) collected on the build host: target `x86_64-unknown-linux-gnu`,
`tests/sim_stub.c` вместо SDL (`sim.c` под x86-linux на хосте не собирается);
квота `LLVM_TRACE_MAX=100000000`, `LLVM_TRACE_EXIT=1`, время < 60 c.
`stats/trace_map_{O2,Os}.txt` — id-соответствие (id → opcode/функция/текст)
того прогона. Инструкция могла отличаться в зависимости от target’а, поэтому
map лежит в `stats/` рядом с результатами.

Итоги (кратко):
- -O2 (100M пар): верхние синглтоны — getelementptr 12.1%, call 10.5%,
  insertelement 10.1%, load 10.0%; топ-2 пары — call<-const (8.0%),
  insertelement<-const (6.8%), getelementptr<-getelementptr (5.7%)
  (векторизация основного цикла + индексная арифметика).
- -Os (100M пар): верхние синглтоны — getelementptr 12.4%, load 10.6%,
  insertelement 9.8%, call 9.6% (векторизованный цикл остался, но индексная
  арифметика тоньше); топ-пары — call<-const (7.4%),
  insertelement<-const (6.6%), getelementptr<-getelementptr (5.1%).
- Наиболее повторяемые паттерны длины 3–5 — цепочки адресной арифметики и
  загрузок (getelementptr → load → getelementptr → load), а также хвост
  цикла вывода (load → fsub → fmul → fptosi → call), каждый с долей ~2.5%.


