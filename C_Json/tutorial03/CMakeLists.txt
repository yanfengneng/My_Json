# cmake 最低版本要求3.0以上
cmake_minimum_required (VERSION 3.0)

# 定义工程名 (需要生成的工程名 工程所支持的语言)
# project (leptjson_test C)

# 设置gcc编译选项，并开启警告wall
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

# 生成库文件 (需要生成的库文件名 源文件...)
add_library(leptjson leptjson.c)

# 生成可执行文件 (生成可执行文件名 源文件...)
add_executable(leptjson_test test.c)

# 将库文件 leptjson 连接到可执行文件 leptjson_text
target_link_libraries(leptjson_test leptjson)