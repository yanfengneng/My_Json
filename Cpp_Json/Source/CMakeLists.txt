cmake_minimum_required(VERSION 3.1.0)

project (Json)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++17 support has been enabled by default.")

# 获取项目目录下所有 .cpp 和 .h 文件的路径，然后将这些文件路径存储在 SOURCE_FILES 变量中
file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

# 生成名为 JSON 的静态库
add_library(${PROJECT_NAME} STATIC  ${SOURCE_FILES})

# 将头文件目录添加到项目中，允许其他项目在使用这个库时能够正确地包含头文件
target_include_directories(${PROJECT_NAME} PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src")
        