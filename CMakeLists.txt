cmake_minimum_required(VERSION 3.15)
project(Emulator)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Debug)

add_executable(Emulator
        Debug/Emulator.pch
        Emulator.cpp
        stdafx.cpp
        stdafx.h
        targetver.h)
