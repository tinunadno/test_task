cmake_minimum_required(VERSION 3.28.3)
project(test_task)

set(CMAKE_CXX_STANDARD 17)


add_executable(test_task
        main.cpp
        file_reader.cpp
        map_processing_exceptions.h
        grid_tracer.cpp
        map_processing.h
        console_interface.cpp
        stored_structures.cpp
        stored_structures.h)
