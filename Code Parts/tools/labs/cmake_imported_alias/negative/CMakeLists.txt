cmake_minimum_required(VERSION 3.18)
project(ExpectedFailure LANGUAGES CXX)
add_executable(missing_target main.cpp)
target_link_libraries(missing_target PRIVATE Missing::Target)
