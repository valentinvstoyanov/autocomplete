cmake_minimum_required(VERSION 3.15)
project(autocomplete)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(autocomplete
        include/autocomplete.h
        include/seeder.h
        src/startup.cc)
