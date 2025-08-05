cmake_minimum_required(VERSION 3.28)
project(NUCC_Parser++ C CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES 
    src/main.cpp
    src/config.cpp
    src/unpacking/xfbin_unpacker.cpp
    src/unpacking/page_unpacker.cpp
    src/repacking/xfbin_repacker.cpp
    src/repacking/page_repacker.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(src/external/cc2_binary_data)
target_link_libraries(${PROJECT_NAME} PRIVATE kojo::cc2_binary_data)