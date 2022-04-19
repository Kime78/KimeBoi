cmake_minimum_required(VERSION 3.23)

project(KimeBoi)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
 SFML
 GIT_REPOSITORY https://github.com/SFML/SFML.git
 GIT_TAG 2.5.1
)
FetchContent_MakeAvailable(SFML)



add_executable(KimeBoi
./src/main.cpp
./src/instructions.cpp
./src/interrupts.cpp ./src/interrupts.hpp
./src/KimeBoi.cpp ./src/KimeBoi.hpp
./src/ppu.cpp ./src/ppu.hpp
./src/timer.cpp ./src/timer.hpp
)

target_link_libraries(KimeBoi sfml-graphics)