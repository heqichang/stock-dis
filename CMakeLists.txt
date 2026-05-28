cmake_minimum_required(VERSION 3.15)
project(DeviceManager VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if(MSVC)
    add_compile_options(/W3 /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_GENERATOR_PLATFORM x64)
    else()
        set(CMAKE_GENERATOR_TOOLSET v141_xp)
    endif()
endif()

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(
    ${SRC_DIR}
    ${THIRD_PARTY_DIR}/sqlite3
    ${THIRD_PARTY_DIR}/zint
    ${THIRD_PARTY_DIR}/nlohmann
    ${THIRD_PARTY_DIR}/libxlsxwriter/include
)

add_subdirectory(${THIRD_PARTY_DIR}/sqlite3)
add_subdirectory(${THIRD_PARTY_DIR}/zint)
add_subdirectory(${THIRD_PARTY_DIR}/libxlsxwriter)

file(GLOB_RECURSE SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.c
    ${SRC_DIR}/*.h
    ${SRC_DIR}/*.hpp
)

add_executable(DeviceManager WIN32 ${SOURCES})

target_link_libraries(DeviceManager
    sqlite3
    zint
    xlsxwriter
    comctl32
    gdi32
    user32
    shell32
)

set_target_properties(DeviceManager PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "设备管理系统"
)
