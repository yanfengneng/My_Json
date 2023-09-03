cmake_minimum_required(VERSION 3.1.0)

project (MiniJsonTest)

# Use C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++17 support has been enabled by default.")

# # 将所有.cpp文件存放在变量TEST_SRC中
# FILE(GLOB_RECURSE TEST_SRC "${CMAKE_SOURCE_DIR}/UnitTest/*.cpp")
# # 将.cpp文件添加到项目中，生成可执行文件
# add_executable(${PROJECT_NAME} ${TEST_SRC})

# # 然后将库连接到目标可执行文件
# target_link_libraries(${PROJECT_NAME} gtest Json -pthread) 

# 以下为采用 gtest 编写代码的生成可执行文件的方法
set(GTEST "${CMAKE_SOURCE_DIR}/UnitTest/gcheck.cpp" "${CMAKE_SOURCE_DIR}/UnitTest/gtest.cpp")
add_executable(MiniJsonGTest ${GTEST})
target_link_libraries(MiniJsonGTest gtest Json -pthread)

# 以下为原生教程编写测试代码的生成可执行文件的方法
set(TEST "${CMAKE_SOURCE_DIR}/UnitTest/test.cpp")
add_executable(MiniJsonTest ${TEST})
target_link_libraries(MiniJsonTest Json)