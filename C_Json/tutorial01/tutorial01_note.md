# 1、JSON 的定义

JSON（JavaScript Object Notation）是一个用于数据交换的文本格式，现时的标准为ECMA-404。

虽然 JSON 源至于 JavaScript 语言，但它只是一种数据格式，可用于任何编程语言。现时具类似功能的格式有 XML、YAML，当中以 JSON 的语法最为简单。

例如，一个动态网页想从服务器获得数据时，服务器从数据库查找数据，然后把数据转换成 JSON 文本格式：
```json
{
    "title": "Design Patterns",
    "subtitle": "Elements of Reusable Object-Oriented Software",
    "author": [
        "Erich Gamma",
        "Richard Helm",
        "Ralph Johnson",
        "John Vlissides"
    ],
    "year": 2009,
    "weight": 1.8,
    "hardcover": true,
    "publisher": {
        "Company": "Pearson Education",
        "Country": "India"
    },
    "website": null
}
```
网页的脚本代码就可以把此 JSON 文本解析为内部的数据结构去使用。

从此例子可看出，JSON 是树状结构，而 JSON 只包含 6 种数据类型：
* null: 表示为 null
* boolean: 表示为 true 或 false
* number: 一般的浮点数表示方式，在下一单元详细说明
* string: 表示为 "..."
* array: 表示为 [ ... ]
* object: 表示为 { ... }

我们要实现的 JSON 库，主要是完成 3 个需求：

1. 把 JSON 文本解析为一个树状数据结构（parse）。
2. 提供接口访问该数据结构（access）。
3. 把数据结构转换成 JSON 文本（stringify）。

![](Image/requirement.png)



# 2、头文件与 API 设计

C 语言有头文件的概念，需要使用 `#include`去引入头文件中的类型声明和函数声明。但由于头文件也可以 `#include` 其他头文件，为避免重复声明，通常会利用宏加入 include 防范。

```c++
#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/* ... */

#endif /* LEPTJSON_H__ */
```

宏的名字必须是唯一的，通常习惯以 `_H__` 作为后缀。由于 leptjson 只有一个头文件，可以简单命名为 `LEPTJSON_H__`。如果项目有多个文件或目录结构，可以用 `项目名称_目录_文件名称_H__` 这种命名方式。

由于 C 语言没有 C++ 的命名空间（namespace）的功能，一般会使用项目的简写作为标识符的前缀。通常枚举值用全大写（如 `LEPT_NULL`），而类型及函数则用小写（如 `lept_type`）。



# 3、单元测试

很多时候我们在做练习题时，都是以 printf／cout 打印结果，再用肉眼对比结果是否乎合预期。但当软件项目越来越复杂，这个做法会越来越低效。一般我们会采用自动的测试方式，例如单元测试（unit testing）。单元测试也能确保其他人修改代码后，原来的功能维持正确（这称为回归测试／regression testing）。

常用的单元测试框架有 xUnit 系列，如 C++ 的 Google Test、C# 的 NUnit。我们为了简单起见，会编写一个极简单的单元测试方式。

一般来说，软件开发是以周期进行的。例如，加入一个功能，再写关于该功能的单元测试。但也有另一种软件开发方法论，称为测试驱动开发（test-driven development, TDD），它的主要循环步骤是：
1. 加入一个测试。

2. 运行所有测试，新的测试应该会失败。

3. 编写实现代码。

4. 运行所有测试，若有测试失败回到3。

5. 重构代码。

6. 回到 1。

TDD 是先写测试，再实现功能。好处是实现只会刚好满足测试，而不会写了一些不需要的代码，或是没有被测试的代码。

但无论我们是采用 TDD，或是先实现后测试，都应尽量加入足够覆盖率的单元测试。



# 4、宏的编写技巧

`反斜线/`表示改行未结束，会串接到下一行。而如果宏里有多过一个语句（statement），就需要用` do { /*...*/ } while(0) `包裹成单个语句。



# 5、关于断言

断言（assertion）是 C 语言中常用的防御式编程方式，减少编程错误。最常用的是在函数开始的地方，检测所有参数。有时候也可以在调用函数后，检查上下文是否正确。

C 语言的标准库含有 `assert()` 这个宏（需 `#include <assert.h>`），提供断言功能。当程序以 release 配置编译时（定义了 `NDEBUG` 宏），`assert()` 不会做检测；而当在 debug 配置时（没定义 `NDEBUG` 宏），则会在运行时检测 `assert(cond)` 中的条件是否为真（非 0），断言失败会直接令程序崩溃。

**如何辨别何时使用断言、何时处理运行时的错误（如返回错误值或在 C++ 中抛出异常）：**

* 简单的答案是：如果那个错误是由于程序员错误编码所造成的（例如传入不合法的参数），那么应用断言；如果那个错误是程序员无法避免，而是由运行时的环境所造成的，就要处理运行时错误（例如开启文件失败）。



# 6、总结

总结：代码实现非常巧妙，用 `lept_value* v` 来获得解析得到的 `json` 字符串类型，用 `lept_parse_value()` 的返回值来判断相应的 `json` 字符串是否解析成功。