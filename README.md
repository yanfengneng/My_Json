- [1. 前置知识](#1-前置知识)
  - [1.1 Git submodule 子模块的管理和使用](#11-git-submodule-子模块的管理和使用)
  - [1.2 编写 CMakeLists.txt 文件](#12-编写-cmakeliststxt-文件)
- [2. 项目解释](#2-项目解释)
- [3. 安装并编译项目](#3-安装并编译项目)
  - [下载项目](#下载项目)
  - [编译项目](#编译项目)
  - [运行测试文件](#运行测试文件)

# 1. 前置知识
## 1.1 [Git submodule 子模块的管理和使用](https://www.jianshu.com/p/9000cd49822c)

添加 gtest 作为 git 子模块：
```bash
# 1. 将远程 gtest 官方库添加到本地文件夹 Cpp_Json_gtest 中
git submodule add https://github.com/google/googletest.git Cpp_Json/gtest

# 2. 添加完子模块后，可以在当前根目录下看到 .gitmodules 的文件，这就表示子模块添加成功了

# 3. 更新子模块为远程项目的最新版本
git submodule update --remote
```
## 1.2 编写 CMakeLists.txt 文件
可参考[cmake中多级CMakeLists.txt调用](https://blog.csdn.net/weixin_42700740/article/details/126364574)、[CMake 学习笔记](https://xiaoneng.blog.csdn.net/article/details/124545500)

# 2. 项目解释
该 JSON 项目主要学习 [miloyip 的从 0 开始写一个 json 解析库](https://github.com/miloyip/json-tutorial)，学习过程持续更新中（2023/6/13）。

`leptjson.h`、`leptjson.c` 是用来解析 json 文件的实现代码，`test.c` 是用来测试编写函数的正确性。

该项目大致想法是先按照教程用 C 语言写一遍，然后使用 C++ 对项目进行重构。
* C_Json（2023/6/13~2023/6/25） 表示按照教程编写的 C 语言代码。
* Cpp_Json（2023/6/26~2023/9/3） 表示使用 C++ 重构之前编写的 C 语言代码。

***
Cpp_Json 将 C_Json 中完成的 C 语言的 Json 库使用 C++ 进行封装与重写，这里主要封装了 5 个类，每个类都使用一个头文件与 .cpp 文件进行实现。解释如下：
* Json 类主要是用提供外部调用的接口，而 JsonValue 是负责实现该接口的（避免内部数据暴露在外）。这里主要是使用一个 std::unique_ptr 的一个指针来实现接口与实现的分离，Json 类调用 JsonValue类的实现，Json 类提供给外部调用接口。
* JsonException 类是将原 c 语言代码中的解析出来的错误信息进行封装，继承 `std::runtime_error` 来抛出异常信息。
* JsonGenerator 类主要是用来实现 json 字符串的生成的。
* Parse 类是用来解析 json 字符串的。
  
本项目使用 `gtest` 来做单元测试，主要参考[轻量级Json库](https://github.com/Syopain/Json) 、[MiniJson](https://github.com/zsmj2017/MiniJson) 。

# 3. 安装并编译项目
## 下载项目
```bash
git clone git@github.com:yanfengneng/My_Json.git
```
## 编译项目
```bash
cd Cpp_Json
mkdir build && cd build
cmake .. && make
```
## 运行测试文件
```bash
UnitTest/MiniJsonTest

UnitTest/MiniJsonGTest
```