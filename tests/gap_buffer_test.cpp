#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "GapBuffer.h"

#include <chrono>
#include <string>

TEST_CASE("GapBuffer Initialization") {
    GapBuffer buf("Hello", 10);
    CHECK(buf.text_size() == 5);
    CHECK(buf.cursor_pos() == 5);
    CHECK(buf.get_text() == "Hello");
}

TEST_CASE("GapBuffer Insert and Delete") {
    GapBuffer buf;
    buf.insert_char('A');
    buf.insert_char('B');
    CHECK(buf.get_text() == "AB");
    buf.delete_char();
    CHECK(buf.get_text() == "A");
}

TEST_CASE("GapBuffer Move Cursor Left and Right") {
    GapBuffer buf("ABC");
    buf.move_cursor(-1); // Move to between B and C
    buf.insert_char('X');
    CHECK(buf.get_text() == "ABXC");
    buf.move_cursor(1); // Move to end
    buf.insert_char('Y');
    CHECK(buf.get_text() == "ABXCY");
}

TEST_CASE("GapBuffer Absolute Cursor Move") {
    GapBuffer buf("12345");
    buf.move_cursor_to(2);
    buf.insert_char('X');
    CHECK(buf.get_text() == "12X345");
}

TEST_CASE("GapBuffer Boundary Conditions") {
    GapBuffer buf;
    buf.delete_char(); // delete on empty buffer
    CHECK(buf.get_text() == "");
    buf.move_cursor(-10); // over move
    CHECK(buf.cursor_pos() == 0);
    buf.move_cursor(10);
    CHECK(buf.cursor_pos() == 0);
}

TEST_CASE("GapBuffer Benchmark: 1MB Insert Latency") {
    // 1MB string
    std::string large_text(1024 * 1024, 'a');
    GapBuffer buf(large_text);
    
    // Move cursor to middle
    buf.move_cursor_to(512 * 1024);
    
    auto start = std::chrono::high_resolution_clock::now();
    buf.insert_char('X');
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    // Must be < 1000 microseconds (1ms)
    CHECK_MESSAGE(duration_us < 1000, "Performance FAIL: Insert latency exceeded 1ms. Actual: ", duration_us, " us");
}

TEST_CASE("GapBuffer Benchmark: 1MB Delete Latency") {
    std::string large_text(1024 * 1024, 'a');
    GapBuffer buf(large_text);
    buf.move_cursor_to(512 * 1024);
    
    auto start = std::chrono::high_resolution_clock::now();
    buf.delete_char();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    CHECK_MESSAGE(duration_us < 1000, "Performance FAIL: Delete latency exceeded 1ms. Actual: ", duration_us, " us");
}
