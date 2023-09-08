cmake_minimum_required(VERSION 3.20)

project(sarch32 VERSION 0.1.0)

include_directories(include)

set(LIB_SOURCE src/sarch.c)
set(C_SOURCE_FILES src/main.c)

add_library(sarch ${LIB_SOURCE})
add_executable(${PROJECT_NAME} ${C_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} sarch)
