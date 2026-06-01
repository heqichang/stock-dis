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
    ${THIRD_PARTY_DIR}/zint/backend
    ${THIRD_PARTY_DIR}/nlohmann
    ${THIRD_PARTY_DIR}/libxlsxwriter/include
)

add_subdirectory(${THIRD_PARTY_DIR}/sqlite3)

# Zint 库配置
set(ZINT_SHARED OFF CACHE BOOL "Build shared library" FORCE)
set(ZINT_STATIC ON CACHE BOOL "Build static library" FORCE)
set(ZINT_FRONTEND OFF CACHE BOOL "Build frontend" FORCE)
set(ZINT_USE_QT OFF CACHE BOOL "Build with Qt support" FORCE)
set(ZINT_USE_PNG OFF CACHE BOOL "Build with PNG support" FORCE)
set(ZINT_USE_GS1SE OFF CACHE BOOL "Build with GS1 Syntax Engine" FORCE)
set(ZINT_TEST OFF CACHE BOOL "Set test compile flag" FORCE)
set(ZINT_UNINSTALL OFF CACHE BOOL "Add uninstall target" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/zint)

add_subdirectory(${THIRD_PARTY_DIR}/libxlsxwriter)

file(GLOB_RECURSE SOURCES
    ${SRC_DIR}/*.cpp
    ${SRC_DIR}/*.c
    ${SRC_DIR}/*.h
    ${SRC_DIR}/*.hpp
)

add_executable(DeviceManager WIN32 ${SOURCES} ${SRC_DIR}/app.manifest)

target_link_libraries(DeviceManager
    sqlite3
    zint-static
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
