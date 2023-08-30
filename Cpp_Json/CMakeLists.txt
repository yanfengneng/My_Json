cmake_minimum_required(VERSION 3.1.0)

project(MiniJson)
# 向构建日志输出当前构建类型的信息
message("current build type is ${CMAKE_BUILD_TYPE}")

# Use C++17
set(CMAKE_CXX_STANDARD 17) # 使用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 设置要求使用指定的 c++ 标准，如果找不到支持所选标准的编译器，cmake 将产生错误
message(STATUS "C++17 support has been enabled by default.") # 默认启用了 C++17 支持

# 添加子目录的编译信息，子目录中必须要有 cmakelists 文件
add_subdirectory("Source")
add_subdirectory("UnitTest")

if (ENABLE_MINIJSON_TEST)
    add_subdirectory("gtest")
    add_subdirectory("UnitTest")
endif()
