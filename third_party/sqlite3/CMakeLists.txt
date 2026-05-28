add_library(sqlite3 STATIC sqlite3.c sqlite3.h)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
