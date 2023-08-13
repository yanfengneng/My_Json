#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

/*
    由于 JSON 是一个树形结构，因此最终我们需要实现一个树的数据结构。
    每个节点使用lept_value的结构体表示，称其为一个json值。
    由于一个 json 值不可能同时为数字和字符串，因此可以使用 union 来节约内存。
    使用之前解析字符串时实现的堆栈，来解决解析 json 数组时未知数组大小的问题。
    使用动态数组来表示键值对的集合。
*/
struct lept_value {
    union {
        /* 每个对象仅仅是成员的数组，与数组无异 */
        struct { lept_member* m; size_t size; }o;   /* object: members, member count */
        struct { lept_value* e; size_t size; }a;    /* array:  elements, element count */
        struct { char* s; size_t len; }s;           /* string: null-terminated string, string length */
        double n;                                   /* number */
    }u;
    lept_type type;
};

/* json 对象和 json 数组的区别在于 json 对象是以花括号 {} 包括起来的，另外 json 对象由对象成员组成，而 json 数组由 json 值组成。
对象成员是指键值对，键必须为 json 字符串，值可以是任何 json 值，中间以冒号分隔。 */
struct lept_member {
    /* key 必须为 json 字符串 */
    char* k; size_t klen;   /* member key string, key string length */
    /* value 值可以是任何 json 值 */
    lept_value v;           /* member value */
};

enum {
    LEPT_PARSE_OK = 0,                      // 解析成功，返回正确码
    /* 以下为解析失败的错误码 */
    LEPT_PARSE_EXPECT_VALUE,                // 只有空白值
    LEPT_PARSE_INVALID_VALUE,               // 解析出无效值
    LEPT_PARSE_ROOT_NOT_SINGULAR,           // 解析的空白之后还有其他字符
    LEPT_PARSE_NUMBER_TOO_BIG,              // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK,         // 缺少引号
    LEPT_PARSE_INVALID_STRING_ESCAPE,       // 无效字符串转义
    LEPT_PARSE_INVALID_STRING_CHAR,         // 无效字符串字符
    LEPT_PARSE_INVALID_UNICODE_HEX,         // 无效 unicode 十六进制 
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,   // 无效 unicode 代理
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, // 遗漏逗号或方括号
    LEPT_PARSE_MISS_KEY,                    // 丢失键
    LEPT_PARSE_MISS_COLON,                  // 丢失冒号
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET  // 丢失逗号或花括号
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

/* 获得数组中元素的个数 */
size_t lept_get_array_size(const lept_value* v);
/* 根据索引 index 来获得某个数组元素 */
lept_value* lept_get_array_element(const lept_value* v, size_t index);

/* 添加最基本的访问函数，用于编写单元测试 */
size_t lept_get_object_size(const lept_value* v);// 获得对象的元素个数
const char* lept_get_object_key(const lept_value* v, size_t index);// 获得对象的 key 值
size_t lept_get_object_key_length(const lept_value* v, size_t index);// 获得对象 key 值的长度
lept_value* lept_get_object_value(const lept_value* v, size_t index);// 获得对象的 value 值

#endif /* LEPTJSON_H__ */