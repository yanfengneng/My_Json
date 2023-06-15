#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */

/* 
由于json的语法特别简单，我们跳过当前字符后，然后检查它的剩余字符就能知道它是那种类型的值。
使用一个宏 EXPECT(c, ch) 进行断言，并跳到下一字符。如 lept_parse_null() 开始时，当前字符应该是 'n'，所以我们使用一个宏 EXPECT(c, ch) 进行断言，并跳到下一字符。
*/
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

// 过滤掉json字符串中的空白，即空格符、制表符、换行符、回车符
static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    // 跳过空白
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

// 实现：t ➔ true
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    // 解析失败，返回无效状态
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    // json的类型设置为true
    v->type = LEPT_TRUE;
    // 解析成功，返回有效状态
    return LEPT_PARSE_OK;
}

// 实现：f ➔ false
static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    // 解析失败，返回无效状态
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    // json的类型设置为false
    v->type = LEPT_FALSE;
    // 解析成功，返回有效状态
    return LEPT_PARSE_OK;
}

// n ➔ null
static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    // 解析失败，返回无效状态
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    // json的类型设置为null
    v->type = LEPT_NULL;
    // 解析成功，返回有效状态
    return LEPT_PARSE_OK;
}

// ture false null、全为空白、无效值的测试结果
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;  // 返回状态：json字符串中只含有空白
        default:   return LEPT_PARSE_INVALID_VALUE; // 返回状态：json字符串的值为无效值
    }
}

/* 
解析json：
    这里的JSON-text = ws value ws。ws 表示空白，value 表示值，因此 JSON 语法表示为空白+值+空白。
    空白是由 0 个或多个空格符、制表符、换行符和回车符所组成。值可以是 null、false 或 true。
    由于 JSON 语法特别简单，我们不需要写分词器（tokenizer），只需检测下一个字符，便可以知道它是哪种类型的值，然后调用相关的分析函数。对于完整的 JSON 语法，跳过空白后，只需检测当前字符.
    若lept_parse()失败，会把v设为null类型，所以这里先把它设为null，让lept_parse_value()写入解析出来的根值。
*/
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret = 0;// 初始化是个好习惯
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
        if (*c.json != '\0')// 空白之后还有值，则将 ret 设置为 空白之后还有其他字符 的错误码。
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

/* 获得 json 值的类型 */
lept_type lept_get_type(const lept_value* v) 
{
    assert(v != NULL);
    return v->type;
}