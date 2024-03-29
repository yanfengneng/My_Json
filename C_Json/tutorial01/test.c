#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;  // 测试总数
static int test_pass = 0;   // 通过总数

/*
解释：
    equality表示expect与actual是否相等，format用来打印错误信息
    表示将测试文件的绝对路径、测试文件的代码行数、预期值、实际值给打印出来。
    fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);
    宏中的反斜线表示该行未结束，还会串接到下一行。
*/
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)


// 使用这个宏时，若 expect!=actual（预期值不等于实际值），便会输出错误信息
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

// static 函数的意思是指，该函数只作用于编译单元中，那么没有被调用时，编译器是能发现的。
// 测试 null
static void test_parse_null() {
    lept_value v;
    v.type = LEPT_FALSE;// 先把数据类型设置为 false
    // 判断字符串"null"能否解析成功，成功则函数 lept_parse() 的返回值为 LEPT_PARSE_OK，否则会返回其他状态值
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    // 判断解析到的类型 v 是否是 null
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

// 实现：测试 true
static void test_parse_true() {
    lept_value v;
    v.type = LEPT_FALSE;
    // 判断
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

// 实现：测试 false
static void test_parse_false() {
    lept_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

// 测试只有空白
static void test_parse_expect_value() {
    lept_value v;

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, ""));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, lept_parse(&v, " "));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

// 测试无效状态
static void test_parse_invalid_value() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "nul"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "?"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

// 测试空白之后还有字符的状态
static void test_parse_root_not_singular() {
    lept_value v;
    v.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

// 测试所有的函数
static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    puts("hello world!");
    return main_ret;
}