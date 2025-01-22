cmake_minimum_required(VERSION 3.30)
project(quiz_c_projekt C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(quiz_c_projekt
    main.c
    main.h)
