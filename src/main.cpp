#include <iostream>
#include "GapBuffer.h"

int main() {
    std::cout << "t-edit: High Performance Markdown Editor Kernel" << std::endl;
    
    GapBuffer buf("Hello World!");
    std::cout << "Initial: " << buf.get_text() << std::endl;
    
    // Move to space
    buf.move_cursor_to(5);
    buf.insert_char(',');
    std::cout << "Inserted comma: " << buf.get_text() << std::endl;
    
    buf.move_cursor(1); // after space Wait, no, cursor was after 'o', space is pos 5. comma is at 5. Space moved to 6. cursor is now 6.
    buf.delete_char(); // deletes comma
    buf.insert_char(' ');
    // let's just make a simple demo.
    
    buf.move_cursor_to(13);
    buf.insert_char('!');
    std::cout << "Final text: " << buf.get_text() << std::endl;

    std::cout << "Current Cursor Pos: " << buf.cursor_pos() << std::endl;
    
    return 0;
}
