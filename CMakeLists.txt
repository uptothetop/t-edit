cmake_minimum_required(VERSION 3.15)
project(t-edit VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure output goes to standard directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main library
add_library(gapbuffer src/GapBuffer.cpp)

# Demo executable
add_executable(t-edit-demo src/main.cpp)
target_link_libraries(t-edit-demo PRIVATE gapbuffer)

# Windows Native UI executable
if(WIN32 OR MINGW)
    add_executable(t-edit-gui WIN32 src/gui_windows/main.cpp)
    target_link_libraries(t-edit-gui PRIVATE gapbuffer d2d1 dwrite windowscodecs user32 gdi32)
    # Statically link to avoid missing DLLs on target machines or Wine
    target_link_options(t-edit-gui PRIVATE "-static" "-static-libgcc" "-static-libstdc++")
endif()

# Testing
option(BUILD_TESTS "Build tests and benchmarks" ON)
if(BUILD_TESTS)
    enable_testing()
    
    add_executable(GapBufferTests tests/gap_buffer_test.cpp)
    target_include_directories(GapBufferTests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    target_link_libraries(GapBufferTests PRIVATE gapbuffer)

    add_test(NAME GapBufferTests COMMAND GapBufferTests)
endif()
