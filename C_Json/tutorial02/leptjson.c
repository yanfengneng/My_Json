#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

/* 为了减少解析函数之间传递多个参数，则将这些数据都放进这个结构体内 */
typedef struct {
    const char* json;
}lept_context;

/* 过滤掉 json 字符串中的空白，即空格符、制表符、换行符、回车符 */
static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* 合并 false、true、null 的解析函数 */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);// 判断 c 的第一个字符是否与 literal[0] 相等。相等就将 c 的 json 字符串右移一位，否则就报错了。
    for (i = 0; literal[i + 1]; i++)// 直到 literal[i+1] 为 '\0'，循环就结束。
        if (c->json[i] != literal[i + 1])// 解析失败，返回无效状态
            return LEPT_PARSE_INVALID_VALUE;
    // 解析成功：将 json 字符串右移，并设置 json 的类型设置为 tpye
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;// 返回解析成功状态
}

/* 将十进制的数字转换为二进制的 double，使用标准库的 strtod() 函数来进行转换。 */
static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;// 使用一个指针 p 来表示当前的解析字符位置
    /* 由于 strtod() 可以转换 json 不容许的格式，因此需要遍历这个 json 数字将不容许的状态直接返回，避免将 json 的十进制数字转换为 double 二进制数字 */
    // 处理负号
    if (*p == '-') p++;
    // 处理整数部分，分为两种合法情况：一种是单个 0，另一种是一个 1~9 再加上任意数量的 digit。
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;// 不是1~9之间的数字，返回解析出来无效值这个状态。
        for (p++; ISDIGIT(*p); p++);// 跳过所有的连续数字
    }
    // 处理小数部分
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;// 不是0~9之间的数字，返回解析出来无效值这个状态。
        for (p++; ISDIGIT(*p); p++);// 跳过所有的连续数字
    }
    // 处理指数部分
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;// 跳过指数的符号
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;// 不是0~9之间的数字，返回解析出来无效值这个状态。
        for (p++; ISDIGIT(*p); p++);// 跳过所有的连续数字
    }
    errno = 0;
    /* 将 json 十进制数字转换为 double 型二进制数字 */
    v->n = strtod(c->json, NULL);
    /* 转换之后的数字太大，返回数字过大这个状态 */
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER; // 更新 json 类型
    c->json = p;// 更新字符串
    return LEPT_PARSE_OK;
}

/* 解析 josn 字符串 */
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

/* 
解析json：
    这里的JSON-text = ws value ws。ws 表示空白，value 表示值，因此 JSON 语法表示为空白+值+空白。
    空白是由 0 个或多个空格符、制表符、换行符和回车符所组成。值可以是 null、false、true 或 number。
    由于 JSON 语法特别简单，我们不需要写分词器（tokenizer），只需检测下一个字符，便可以知道它是哪种类型的值，然后调用相关的分析函数。对于完整的 JSON 语法，跳过空白后，只需检测当前字符.
    若lept_parse()失败，会把 v 设为null类型，所以这里先把它设为 null，让 lept_parse_value() 写入解析出来的根值。
*/
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret=0;// 初始化是个好习惯
    assert(v != NULL);
    c.json = json;
    // 若 lept_parse() 失败，会把 v 设为 null 类型，所以这里先把它设为 null，让 lept_parse_value() 写入解析出来的根值。
    v->type = LEPT_NULL;
    // 过滤json字符串中的空白
    lept_parse_whitespace(&c);
     // 开始测试json字符串是否合法
    // 解析成功的话，就将 ret 的值设置为 LEPT_PARSE_OK，然后跳过值后面的空白。
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);// 跳过值后面的空白
        // json 在一个值之后，空白之后还有其它字符，则要返回 LEPT_PARSE_ROOT_NOT_SINGULAR。
        if (*c.json != '\0') {
            v->type = LEPT_NULL;// 将 json 类型设置为 null
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;// 空白之后还有值，则将 ret 设置为 空白之后还有其他字符 的错误码。
        }
    }
    return ret;
}

/* 获得 json 值的类型 */
lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

/* 获得解析出来的数字 */
double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}