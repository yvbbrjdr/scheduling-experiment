cmake_minimum_required(VERSION 3.7.2)
project(se C)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
add_definitions(-Wall -Wextra)
add_executable(se
    src/blockingthread.c
    src/epollthread.c
    src/main.c
    src/mutexthread.c
    src/threadcontext.c
    src/utils.c)
find_package(Threads REQUIRED)
target_link_libraries(se ${CMAKE_THREAD_LIBS_INIT})
