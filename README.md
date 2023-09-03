- [1. 项目解释](#1-项目解释)
- [2. 安装并编译项目](#2-安装并编译项目)
  - [下载项目](#下载项目)
  - [编译项目](#编译项目)
  - [运行测试文件](#运行测试文件)


# 1. 项目解释
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

# 2. 安装并编译项目
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