cmake_minimum_required(VERSION 3.20)

project(particle_sandbox LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib REQUIRED)

add_executable(particle_sandbox
    src/main.cpp
    src/ParticleSystem.cpp
)

target_include_directories(particle_sandbox PRIVATE include external)

target_link_libraries(particle_sandbox PRIVATE raylib)
