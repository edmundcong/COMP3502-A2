cmake_minimum_required(VERSION 3.6)
project(assignment_2)

set(CMAKE_C_STANDARD 99)

#set(SOURCE_FILES main.c cmake-build-debug/pcb.h mab.h cmake-build-debug/hosted.h cmake-build-debug/hosted.c hostd.c hostd.h)
set(SOURCE_FILES main.c cmake-build-debug/pcb.h hostd.c hostd.h)
add_executable(assignment_2 ${SOURCE_FILES})