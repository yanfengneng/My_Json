#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
// 判断字符串 c 中压入的字符是否和字符 ch 是相等的
#define PUTC(c, ch)         do { *(char*)lept_context_push(c, sizeof(char)) = (ch); } while(0)

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;// size 表示当前堆栈的容量，top 表示栈顶的位置（由于 stack 会被扩展，因此不需要把 top 以指针的形式存储）
}lept_context;

/* 压栈：将数组压入栈中。压入任意大小的数据 */
static void* lept_context_push(lept_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    // 若栈的大小不够，也就是栈顶的位置加上压入数据的大小大于等于当前栈的容量，需要扩展栈的容量
    if (c->top + size >= c->size) {
        if (c->size == 0)// 若栈的大小为 0，则将其容量设置为初始值
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        // 栈顶的位置加上数据的大小大于等于栈的容量时，也就是栈的容量不够时，将栈以1.5倍大小进行扩展，也就是每次将栈的容量加上0.5倍，直到栈的容量能满足数据插入为止
        while (c->top + size >= c->size)// c->size >> 1 表示加上 0.5 倍
            c->size += c->size >> 1;  /* c->size * 1.5 */
        // 现在将为 stack 分配大小为 c->size 的内存空间
        c->stack = (char*)realloc(c->stack, c->size);// c->stack 在初始化时为 NULL，realloc(NULL, size) 的行为是等价于 malloc(size) 的，所以不需要为第一次分配内存作特别处理。
    }
    // ret 保留数据的起始指针，栈在内存空间的位置加上原来栈顶的大小，为压入数据的起始位置
    ret = c->stack + c->top;
    // 栈顶的新位置为原来栈顶位置加上数据的大小
    c->top += size;
    return ret;
}

/* 弹出任意大小的数据 */
static void* lept_context_pop(lept_context* c, size_t size) {
    // assert(int expression)，若 expression = 0，则终止程序，并打印错误信息；若 expression != 0，则断言不会触发。
    // 也就是说只有断言里面的表达式的结果为 true 时，则不会抛出异常；表示式的结果为 false 时，会抛出异常，并打印错误信息。
    // 所以栈顶的位置只有大于等于数据的大小时，才能弹出 size 大小的数据
    assert(c->top >= size);
    // 返回栈顶的位置
    return c->stack + (c->top -= size);
}

/* 处理空白 */
static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* 合并 false、true、null 的解析函数 */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
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
    v->u.n = strtod(c->json, NULL);
    /* 转换之后的数字太大，返回数字过大这个状态 */
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER; // 更新 json 类型
    c->json = p;// 更新字符串
    return LEPT_PARSE_OK;
}

/* 解析字符串 */
static int lept_parse_string(lept_context* c, lept_value* v) {
    size_t head = c->top, len;
    const char* p;
    EXPECT(c, '\"');// 跳过第一个双引号
    p = c->json;
    for (;;) {
        char ch = *p++;// 获得当前字符
        switch (ch) {
            case '\"':// 字符串的双引号
                len = c->top - head;// 获得栈中字符串的长度
                lept_set_string(v, (const char*)lept_context_pop(c, len), len);// 将字符串 c 复制到 v 中
                c->json = p;// 更新 c 中字符串的起始位置
                return LEPT_PARSE_OK;// 返回解析成功的标志
            case '\\':// 字符串的转义符
                switch (*p++) {
                    // 8 种转义字符
                    case '\"': PUTC(c, '\"'); break;// 引号
                    case '\\': PUTC(c, '\\'); break;// 反斜线
                    case '/':  PUTC(c, '/' ); break;// 斜线
                    case 'b':  PUTC(c, '\b'); break;// 退格键
                    case 'f':  PUTC(c, '\f'); break;// 换页
                    case 'n':  PUTC(c, '\n'); break;// 换行
                    case 'r':  PUTC(c, '\r'); break;// 回车
                    case 't':  PUTC(c, '\t'); break;// tab 键
                    default:
                        c->top = head;// 还原栈顶位置
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;// 返回无效字符转义状态
                }
                break;
            case '\0':// 结束符
                c->top = head;// 还原栈顶位置
                return LEPT_PARSE_MISS_QUOTATION_MARK;// 返回缺少引号状态
            // 当中空缺的 %x22 是双引号，%x5C 是反斜线，都已经处理。所以不合法的字符是 %x00 至 %x1F。我们简单地在 default 里处理。
            default:
                if ((unsigned char)ch < 0x20) { 
                    c->top = head;// 还原栈顶位置
                    return LEPT_PARSE_INVALID_STRING_CHAR;// 返回无效字符串状态
                }
                // 将 c 移动到下一个位置
                PUTC(c, ch);
        }
    }
}

/* 解析 josn 字符串的值 */
static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);// 默认方式是解析数字
        case '"':  return lept_parse_string(c, v);// 解析字符串
        case '\0': return LEPT_PARSE_EXPECT_VALUE;// 返回只有空白值状态
    }
}

/* 解析 json 字符串 */
int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret=0;// 初始化是个好习惯
    assert(v != NULL);
    c.json = json;
    /* 初始化栈 */
    c.stack = NULL;
    c.size = c.top = 0;
    /* 初始化 v 的类型为 null */
    lept_init(v);
    // 跳过空白
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {// 判断是否解析成功
        lept_parse_whitespace(&c);// 跳过空白
        // json 在一个值之后，空白之后还有其它字符，则要返回 LEPT_PARSE_ROOT_NOT_SINGULAR。
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    // 释放栈的内存
    assert(c.top == 0);// 释放内存，栈不能为空
    free(c.stack);
    return ret;
}

void lept_free(lept_value* v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING)// 当前值是字符串类型时，我们才释放内存
        free(v->u.s.s);
    v->type = LEPT_NULL;// 释放内存之后需要将它的类型变为 null，这样能避免重复释放
}

/* 获得 json 值的类型 */
lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

/* 判断 v 的类型是否为 true */
int lept_get_boolean(const lept_value* v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
}

/* 根据 b 的值来设置 v 是 true 还是 false */
void lept_set_boolean(lept_value* v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

/* 获得解析出来的数字 */
double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

/* 将浮点数 n 写入 v 中 */
void lept_set_number(lept_value* v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

/* 获得解析出来的字符串 */
const char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

/* 获得解析出来的字符串长度 */
size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    // 非空指针（有具体的字符串）或是零长度的字符串都是合法的
    assert(v != NULL && (s != NULL || len == 0));
    // 清空 v 可能分配到的内存
    lept_free(v);
    // 然后申请内存，并将字符串 s 复制到 v 中，并设置 v 的类型为字符串
    v->u.s.s = (char*)malloc(len + 1);// len+1 中的 1 是因为结尾空字符
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';// 补上结尾空字符
    v->u.s.len = len;
    v->type = LEPT_STRING;
}