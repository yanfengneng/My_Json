cmake_minimum_required (VERSION 3.0)
project (leptjson_test C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()

# 生成库文件
add_library(leptjson leptjson.c)
# 生成可执行文件
add_executable(leptjson_test test.c)
target_link_libraries(leptjson_test leptjson)