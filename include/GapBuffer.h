#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cstddef>

class GapBuffer {
public:
    // Initialize with a given string and default gap size
    GapBuffer(std::string_view initial_text = "", size_t initial_gap_size = 65536);

    // Insert a character at current cursor position
    void insert_char(char c);

    // Delete a character before the cursor (backspace)
    void delete_char();

    // Move cursor relative to current position (negative = left, positive = right)
    void move_cursor(int delta);

    // Move cursor to absolute position
    void move_cursor_to(size_t pos);

    // Get current cursor position
    size_t cursor_pos() const;

    // Get total size of the text
    size_t text_size() const;

    // Return current text (warning: requires copying and allocating a new string)
    std::string get_text() const;

private:
    std::vector<char> buffer;
    size_t gap_start;
    size_t gap_end;

    void expand_gap();
};
