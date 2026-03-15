---
name: "C++ SDET & Performance Testing"
description: "Инструкции по сборке, тестированию памяти и производительности ядра GapBuffer."
---

# Навык: SDET, тестирование и бенчмарки на C++ (doctest)

Этот файл содержит конкретные инструкции и CLI-команды для валидации кода `GapBuffer` согласно правилам `AGENTS.md`. 
Когда требуется запуск тестов, следуйте шагам ниже. 

## Структура тестов
Тесты размещаются в директории `tests/` с использованием `doctest.h`. Должны присутствовать как проверки корректности (Unit Tests), так и замеры времени (Benchmarks).

## 1. Сборка и запуск тестов с санитайзерами (ASAN + UBSAN)

Этот тип сборки используется для проверки безопасности памяти.

```bash
# 1. Создать директорию для отладочной сборки
mkdir -p build_tests
cd build_tests

# 2. Выполнить конфигурацию CMake.
# Необходимо передать флаги AddressSanitizer и UndefinedBehaviorSanitizer.
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -g" \
      -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
      ..

# 3. Скомпилировать тесты
cmake --build .

# 4. Запустить тесты
# Если есть утечки памяти или выход за границы, ASAN остановит выполнение с ошибкой.
./tests/GapBufferTests
```

## 2. Сборка и запуск тестов производительности (Benchmarks)

Для тестирования производительности необходимо собрать код в Release-режиме (с оптимизацией `-O3` или `-O2`), без санитайзеров, чтобы они не искажали замеры времени.

```bash
# 1. Создать директорию для релизной сборки
mkdir -p build_perf
cd build_perf

# 2. Сконфигурировать CMake (Release)
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. Скомпилировать
cmake --build .

# 4. Запустить бенчмарк и проверить время
# УБЕДИТЕСЬ, что время вставки 1 символа в 1МБ текст меньше 1 мс.
./tests/GapBufferTests --test-case="Benchmark*"
```

## Условие провала бенчмарка (Fail Condition)
При написании теста производительности используйте `<chrono>`, замеряйте `std::chrono::high_resolution_clock::now()` до и после операции. 
```cpp
auto start = std::chrono::high_resolution_clock::now();
buffer.insert_char('x');
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

// Пример проверки через doctest
CHECK_MESSAGE(duration < 1000, "Performance FAIL: Insert latency exceeded 1ms. Actual: ", duration, " us");
```
