cmake_minimum_required(VERSION 3.23)
project(compYler)

set(CMAKE_CXX_STANDARD 11)

add_executable(compYler main.cpp Parser.cpp)
include_directories(./include)
