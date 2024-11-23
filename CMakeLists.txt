cmake_minimum_required(VERSION 3.26)
project(hw2 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(hw2
        csapp.c
        csapp.h
        echoclient.c
        echoserveri.c
        echo.c)
