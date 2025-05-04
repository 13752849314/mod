cmake_minimum_required(VERSION 3.22)
project(mod)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

add_executable(mod
        #        main.cpp
        #        lib/game.cpp
        #        lib/game.h
        #        lib/game.h
        #        lib/shared_ptr.cpp
        #        lib/shared_ptr.h
        #        lib/shared_ptr.h
        #        lib/timer.cpp
        #        lib/timer.h
        #        lib/ThreadPool.cpp
        #        lib/ThreadPool.h
        #        test_threadPool.cpp
                Thread.cpp
)

target_link_libraries(mod pthread)