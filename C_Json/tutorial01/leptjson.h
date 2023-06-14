// 使用宏来加入 include 防范
// 宏的名字必须是唯一的，通常习惯以 _H__ 作为后缀
// 若项目有多个文件或目录结构，可以用「项目名称_目录_文件名称_H__」这种命名方式
#ifndef LEPTJSON_H__
#define LEPTJSON_H__

// 我们要解析的 JSON 文件中有6中数据类型，如果把 true 和 false 当作两种类型就是 7 种了，因此声明一个枚举类型
// 由于 C 语言没有 C++ 的命名空间，因此一般会使用项目的简写作为标识符的前缀。通常枚举值用全大写，而类型及函数则用小写。
// 枚举值是常量，默认依次从0开始赋值
typedef enum { 
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT 
}lept_type;

// 由于 JSON 是一个树形结构，因此最终我们需要实现一个树的数据结构
// 每个节点使用lept_value的结构体表示，称其为一个json值
typedef struct {
    lept_type type;
}lept_value;

// 以下枚举值为解析 JSON 函数的返回值，第一个为正确码
// 之后的3个枚举值为错误码：expect_value 表示json字符串中只含有空白、not_singular表示空白之后还有其他字符、invalid_value表示无效值
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

// 为了减少解析函数之间传递多个参数，则将这些数据都放进这个结构体内
typedef struct {
    const char* json;
}lept_context;

// 解析 JSON
int lept_parse(lept_value* v, const char* json);

// 访问结果
lept_type lept_get_type(const lept_value* v);
#endif 