#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/*
    我们要解析的 JSON 文件中有 6 种数据类型，如果把 true 和 false 当作两种类型就是 7 种了，因此声明一个枚举类型。
    由于 C 语言没有 C++ 的命名空间，因此一般会使用项目的简写作为标识符的前缀。通常枚举值用全大写，而类型及函数则用小写。
    枚举值是常量，默认依次从 0 开始赋值。
*/
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/*
    由于 JSON 是一个树形结构，因此最终我们需要实现一个树的数据结构。
    每个节点使用lept_value的结构体表示，称其为一个json值。
*/
typedef struct {
	double n;           // 当 tepe==LEPT_NUMBER 时，n 才表示JSON 数字的数字
    lept_type type;     // 用来存储解析出来的数据类型
}lept_value;

/*
以下枚举值为解析 JSON 函数的返回值：
    LEPT_PARSE_OK 表示解析成功，返回正确码。
    4 个枚举值为错误码：EXPECT_VALUE 表示 json 字符串中只含有空白、ROOT_NOT_SINGULAR 表示空白之后还有其他字符、INVALID_VALUE 表示无效值、NUMBER_TOO_BIG 表示浮点数溢出
*/
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

/* 解析 json：该函数的返回值表示 json 字符串的解析结果，用 v 来保存 json 字符串解析出来的数据类型 */
int lept_parse(lept_value* v, const char* json);

/* 获得解析出来的数据类型 */
lept_type lept_get_type(const lept_value* v);

/* 获得解析出来的数字 */
double lept_get_number(const lept_value* v);

#endif /* LEPTJSON_H__ */