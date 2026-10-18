cmake_minimum_required(VERSION 3.16)
project(bdd_einstein_solver LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_ASAN "Собрать с AddressSanitizer и UBSan" OFF)

# BuDDy — библиотека бинарных решающих диаграмм. Скачивается при конфигурации,
# ставить отдельно не нужно. Версия зафиксирована коммитом.
include(FetchContent)
set(BUDDY_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  buddy
  GIT_REPOSITORY https://github.com/SSoelvsten/buddy.git
  GIT_TAG        5aca063a4b2e90352480f3dd24daeb6dbefa2d33
)
FetchContent_MakeAvailable(buddy)

add_executable(bdd_einstein src/main.cpp)
target_link_libraries(bdd_einstein PRIVATE buddy)

if(ENABLE_ASAN)
  target_compile_options(bdd_einstein PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g)
  target_link_options(bdd_einstein PRIVATE -fsanitize=address,undefined)
endif()
