cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 11)

project(sarch32 VERSION 0.1.0)

include_directories(include)

set(LIB_SOURCE src/sarch.c src/sarch_fetch.c)

add_library(sarch ${LIB_SOURCE})
add_executable(test_exec src/main.c)
target_link_libraries(test_exec sarch)

add_executable(${PROJECT_NAME} src/computer_main.cpp src/computer.cpp)
target_link_libraries(${PROJECT_NAME} sarch)
