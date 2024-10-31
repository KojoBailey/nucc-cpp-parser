cmake_minimum_required(VERSION 3.28)
project(NUCC_Parser_PlusPlus C CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(src/external/nucc)
target_link_libraries(${PROJECT_NAME} PRIVATE nucc-cpp-library)