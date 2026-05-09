cmake_minimum_required(VERSION 3.20)
project(FortWall LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_compile_options(/kernel /GS /GR- /EHsc)
add_link_options(/kernel /driver /ENTRY:DriverEntry /SUBSYSTEM:NATIVE)

add_library(wallhack.sys SHARED wallhack.c)
