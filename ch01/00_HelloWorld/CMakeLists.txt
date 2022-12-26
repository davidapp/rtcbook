cmake_minimum_required(VERSION 3.18.1)

project(HelloWorld)

include_directories(../../Common)

add_executable(Hello main.cpp
            ../../Common/Base/DXP.cpp)