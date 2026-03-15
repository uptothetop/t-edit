#include "GapBuffer.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

GapBuffer::GapBuffer(std::string_view initial_text, size_t initial_gap_size) {
    buffer.resize(initial_text.size() + initial_gap_size);
    
    // Copy initially before the gap
    if (!initial_text.empty()) {
        std::memcpy(buffer.data(), initial_text.data(), initial_text.size());
    }
    
    gap_start = initial_text.size();
    gap_end = buffer.size();
}

void GapBuffer::insert_char(char c) {
    if (gap_start == gap_end) {
        expand_gap();
    }
    buffer[gap_start++] = c;
}

void GapBuffer::delete_char() {
    if (gap_start > 0) {
        gap_start--;
    }
}

void GapBuffer::move_cursor(int delta) {
    if (delta == 0) return;
    
    if (delta < 0) { // Move left
        size_t move_count = std::min(static_cast<size_t>(-delta), gap_start);
        for (size_t i = 0; i < move_count; ++i) {
            buffer[--gap_end] = buffer[--gap_start];
        }
    } else { // Move right
        size_t right_chars = buffer.size() - gap_end;
        size_t move_count = std::min(static_cast<size_t>(delta), right_chars);
        for (size_t i = 0; i < move_count; ++i) {
            buffer[gap_start++] = buffer[gap_end++];
        }
    }
}

void GapBuffer::move_cursor_to(size_t pos) {
    if (pos == gap_start) return;
    
    if (pos < gap_start) {
        move_cursor(-static_cast<int>(gap_start - pos));
    } else {
        move_cursor(static_cast<int>(pos - gap_start));
    }
}

size_t GapBuffer::cursor_pos() const {
    return gap_start;
}

size_t GapBuffer::text_size() const {
    return buffer.size() - (gap_end - gap_start);
}

std::string GapBuffer::get_text() const {
    std::string result;
    result.reserve(text_size());
    result.append(buffer.data(), gap_start);
    result.append(buffer.data() + gap_end, buffer.size() - gap_end);
    return result;
}

void GapBuffer::expand_gap() {
    // Double the buffer size, or add at least 64KB
    size_t new_gap_size = std::max<size_t>(65536, buffer.size());
    size_t old_size = buffer.size();
    buffer.resize(buffer.size() + new_gap_size);
    
    // Move the text after the gap to the end of the new buffer
    size_t right_len = old_size - gap_end;
    if (right_len > 0) {
        std::memmove(buffer.data() + buffer.size() - right_len, 
                     buffer.data() + gap_end, 
                     right_len);
    }
    
    gap_end = buffer.size() - right_len;
}
