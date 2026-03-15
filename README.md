# t-edit: Minimalist & High-Performance Markdown Editor

`t-edit` — это ультра-быстрый Markdown редактор, спроектированный специально для Windows 10 планшетов со слабым процессором (Intel Atom x5-Z8550) и медленным eMMC накопителем. 

Главный принцип проекта: **Нулевая задержка ввода (typing latency)** и минимальное потребление системных ресурсов (ОЗУ/CPU).

## Особенности архитектуры
- **Language**: C++17 (без тяжелых библиотек вроде Boost).
- **Core Data Structure**: **Gap Buffer** — структура, обеспечивающая вставку, удаление и смещение символов локально со скоростью `O(1)`. Это позволяет редактировать огромные файлы без лагов.
- **Build System**: CMake с поддержкой как нативной кросс-компиляции на macOS/Linux с использованием MinGW, так и сборки на Windows.

## Разработка и тестирование (SDET & Performance)
В проекте введены строгие правила проверки качества кода и производительности (см. `AGENTS.md` и `SKILLS.md`).

### Требования для прохождения тестов:
1. Юнит-тесты на `doctest` должны выполняться на 100%.
2. Код должен собираться с AddressSanitizer (ASAN) и UndefinedBehaviorSanitizer (UBSAN) для исключения утечек памяти и выходов за пределы буфера.
3. **Бенчмарк производительности**: вставка 1 символа в текст размером 1 МБ должна занимать строго **меньше 1 миллисекунды (1 ms)** на macOS в релизной сборке.

## Команды для разработчика

Все команды предполагают, что вы находитесь в корне репозитория (в директории `t-edit`).

### 1. Тестирование корректности работы и памяти (ASAN + UBSAN)
Эта конфигурация обязательна перед любым коммитом, изменяющим логику `GapBuffer`.

```bash
mkdir -p build_tests && cd build_tests
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" \
      -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
      ..
cmake --build .

# Запуск тестов. Остановка с ошибкой в случае утечки памяти.
./tests/GapBufferTests
```

### 2. Бенчмарки производительности
Тестирование времени выполнения на релизной сборке (без санитайзеров, чтобы не было накладных расходов).

```bash
mkdir -p build_perf && cd build_perf
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Проверка, что задержка вставки < 1 мс
./tests/GapBufferTests --test-case="Benchmark*"
```

### 3. Сборка для Windows 10 (кросс-компиляция с macOS через MinGW)

Требуется установленный MinGW: `brew install mingw-w64`.

```bash
mkdir -p build_mingw && cd build_mingw
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# В результате в папке будет сгенерирован t-edit-demo.exe
```

## Структура проекта
- `include/` — заголовочные файлы бизнес-логики (`GapBuffer.h`).
- `src/` — реализация ядра (`GapBuffer.cpp`, `main.cpp`).
- `tests/` — юнит-тесты и бенчмарки на `doctest`.
- `cmake/` — файлы toolchain для кросс-компиляции.
