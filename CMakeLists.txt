cmake_minimum_required(VERSION 3.28)
project(NUCC_Parser++ C CXX)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(src/external/nucc)
target_link_libraries(${PROJECT_NAME} PRIVATE kojo::nucc)

add_subdirectory(src/external/cc2_binary_data)
target_link_libraries(${PROJECT_NAME} PRIVATE kojo::cc2_binary_data)