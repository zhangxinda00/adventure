cmake_minimum_required(VERSION 3.10)
project(adventrue_terminal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(adventrue
  src/main.cpp
  src/platform/terminal.cpp
  src/engine/game_loop.cpp
  src/game/physics.cpp
  src/game/player.cpp
  src/game/monster.cpp
  src/game/world.cpp
  src/render/renderer.cpp
)

target_include_directories(adventrue PRIVATE src)
