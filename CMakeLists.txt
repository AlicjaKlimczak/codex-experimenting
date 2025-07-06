cmake_minimum_required(VERSION 3.10)
project(RetroDogeon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(retro_dungeon
    src/main.cpp
    src/textadventure.cpp
    src/room.cpp
    src/room_factory.cpp
)

target_link_libraries(retro_dungeon raylib)

target_include_directories(retro_dungeon PRIVATE include)