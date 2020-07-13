cmake_minimum_required(VERSION 3.2)
project(tty-test-client)

SET (CMAKE_C_COMPILER_WORKS 1)
SET (CMAKE_CXX_COMPILER_WORKS 1)

add_executable(tty-test-client
    main.cpp
    serial_linux.cpp
    tty_tester.cpp
)