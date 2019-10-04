cmake_minimum_required(VERSION 3.13)
project(cpp_threadpool)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(Threads REQUIRED)


add_executable(cpp_threadpool
        src/main.cpp
        include/ThreadPool.h include/ThreadedExecutionHelper.h)

target_link_libraries(cpp_threadpool Threads::Threads)