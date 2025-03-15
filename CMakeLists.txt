cmake_minimum_required(VERSION 3.28.3)
project(test_task)

set(CMAKE_CXX_STANDARD 17)


add_executable(test_task
        main.cpp
        map_processing/IO/file_reader.cpp
        map_processing/map_processing_exceptions.h
        map_processing/grid_processing/grid_tracer.cpp
        map_processing/map_processing.h
        map_processing/UI/console_interface.cpp
        map_processing/stored_structures/stored_structures.cpp
        map_processing/stored_structures/stored_structures.h
        map_processing/string_utils.h)
