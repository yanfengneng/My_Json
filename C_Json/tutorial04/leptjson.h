#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

/*
    我们要解析的 JSON 文件中有 6 种数据类型，如果把 true 和 false 当作两种类型就是 7 种了，因此声明一个枚举类型。
    由于 C 语言没有 C++ 的命名空间，因此一般会使用项目的简写作为标识符的前缀。通常枚举值用全大写，而类型及函数则用小写。
    枚举值是常量，默认依次从 0 开始赋值。
*/
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/*
    由于 JSON 是一个树形结构，因此最终我们需要实现一个树的数据结构。
    每个节点使用lept_value的结构体表示，称其为一个json值。
    由于一个 json 值不可能同时为数字和字符串，因此可以使用 union 来节约内存。
*/
typedef struct {
    union {
        struct { char* s; size_t len; }s;  /* string */
        double n;                          /* number */
    }u;
    lept_type type;
}lept_value;

/* 以下枚举值为解析 JSON 函数的返回值 */
enum {
    LEPT_PARSE_OK = 0,                  // 解析成功，返回正确码
    /* 以下为解析失败的错误码 */
    LEPT_PARSE_EXPECT_VALUE,            // 只有空白值
    LEPT_PARSE_INVALID_VALUE,           // 解析出无效值
    LEPT_PARSE_ROOT_NOT_SINGULAR,       // 解析的空白之后还有其他字符
    LEPT_PARSE_NUMBER_TOO_BIG,          // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK,     // 缺少引号
    LEPT_PARSE_INVALID_STRING_ESCAPE,   // 无效字符串转义
    LEPT_PARSE_INVALID_STRING_CHAR,     // 无效字符串字符
    LEPT_PARSE_INVALID_UNICODE_HEX,     // 无效 unicode 十六进制 
    LEPT_PARSE_INVALID_UNICODE_SURROGATE // 无效 unicode 代理
};

/* 由于我们会检查 v 的类型，所以在调用所有函数之前，必须初始化该类型 */
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

/* 解析 json：该函数的返回值表示 json 字符串的解析结果，用 v 来保存 json 字符串解析出来的数据类型 */
int lept_parse(lept_value* v, const char* json);

/* 释放 v 的内存 */
void lept_free(lept_value* v);

/* 获得解析出来的数据类型 */
lept_type lept_get_type(const lept_value* v);

/* 将 v 的值变成 null 值 */
#define lept_set_null(v) lept_free(v)

/* 判断 v 的类型是否为true */
int lept_get_boolean(const lept_value* v);
/* 设置 v 的 bool 类型 */
void lept_set_boolean(lept_value* v, int b);

/* 获得解析出来的浮点数 */
double lept_get_number(const lept_value* v);
/* 设置 json 中的浮点数 */
void lept_set_number(lept_value* v, double n);

/* 获得解析出来的字符串 */
const char* lept_get_string(const lept_value* v);
/* 获得解析出来字符串的长度 */
size_t lept_get_string_length(const lept_value* v);
/* 将字符串复制到 v 中 */
void lept_set_string(lept_value* v, const char* s, size_t len);

#endif /* LEPTJSON_H__ */
