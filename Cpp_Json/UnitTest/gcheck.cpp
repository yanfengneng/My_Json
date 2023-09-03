#include <gtest/gtest.h>
#include "../Source/include/json.h"
#include <string>

using namespace std;
using namespace yfn;

static std::string status;

#define test_literal(expect, content)\
    do {\
        yfn::Json j;\
        j.set_boolean(false);\
        j.parse(content, status);\
        EXPECT_EQ("parse ok", status);\
        EXPECT_EQ(expect, j.get_type());\
    } while(0)

TEST(TestLiteral,NullTrueFalse)
{
    test_literal(json::Null, "null");
    test_literal(json::True, "true");
    test_literal(json::False, "false");
}

#define test_number(expect, content)\
    do {\
        yfn::Json j;\
        j.parse(content, status);\
        EXPECT_EQ("parse ok", status);\
        EXPECT_EQ(json::Number, j.get_type());\
        EXPECT_EQ(expect, j.get_number());\
    }while(0)

TEST(TestNumber,Number)
{
    test_number(0.0, "0");
	test_number(0.0, "-0");
	test_number(0.0, "-0.0");
	test_number(1.0, "1");
	test_number(-1.0, "-1");
	test_number(1.5, "1.5");
	test_number(-1.5, "-1.5");
	test_number(3.1416, "3.1416");
	test_number(1E10, "1E10");
	test_number(1e10, "1e10");
	test_number(1E+10, "1E+10");
	test_number(1E-10, "1E-10");
	test_number(-1E10, "-1E10");
	test_number(-1e10, "-1e10");
	test_number(-1E+10, "-1E+10");
	test_number(-1E-10, "-1E-10");
	test_number(1.234E+10, "1.234E+10");
	test_number(1.234E-10, "1.234E-10");
	test_number(0.0, "1e-10000"); /* must underflow */
}

#define test_string(expect, content)\
    do {\
        yfn::Json j;\
        j.parse(content, status);\
        EXPECT_EQ("parse ok", status);\
        EXPECT_EQ(json::String, j.get_type());\
        EXPECT_STREQ(expect, j.get_string().c_str());\
    }while(0)

TEST(TestString, String)
{
    test_string("", "\"\"");
	test_string("Hello", "\"Hello\"");

	test_string("Hello\nWorld", "\"Hello\\nWorld\"");
	test_string("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

	test_string("Hello\0World", "\"Hello\\u0000World\"");
	test_string("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	test_string("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	test_string("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	test_string("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	test_string("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

TEST(TestArray, Array)
{
	yfn::Json j;

	j.parse("[ ]", status);
	EXPECT_EQ("parse ok", status);
	EXPECT_EQ(json::Array, j.get_type());
	EXPECT_EQ(0, j.get_array_size());

	j.parse("[ null , false , true , 123 , \"abc\" ]", status);
	EXPECT_EQ("parse ok", status);
	EXPECT_EQ(json::Array, j.get_type());
	EXPECT_EQ(5, j.get_array_size());
	EXPECT_EQ(json::Null, j.get_array_element(0).get_type());
	EXPECT_EQ(json::False, j.get_array_element(1).get_type());
	EXPECT_EQ(json::True, j.get_array_element(2).get_type());
	EXPECT_EQ(json::Number, j.get_array_element(3).get_type());
	EXPECT_EQ(json::String, j.get_array_element(4).get_type());
	EXPECT_EQ(123.0, j.get_array_element(3).get_number());
	EXPECT_STREQ("abc", j.get_array_element(4).get_string().c_str());

	j.parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", status);
	EXPECT_EQ("parse ok", status);
	EXPECT_EQ(json::Array, j.get_type());
	EXPECT_EQ(4, j.get_array_size());
	for(int i = 0; i< 4; i++) {
		yfn::Json a = j.get_array_element(i);
		EXPECT_EQ(json::Array, a.get_type());
		for(int k = 0; k < i; k++) {
			yfn::Json b = a.get_array_element(k);
			EXPECT_EQ(json::Number, b.get_type());
			EXPECT_EQ((double)k, b.get_number());
		}
	}
}

TEST(TestObject, Object)
{
	yfn::Json v;
	
	v.parse(" { } ", status);
	EXPECT_EQ("parse ok", status);
	EXPECT_EQ(json::Object, v.get_type());
	EXPECT_EQ(0, v.get_object_size());

	v.parse(" { "
	        "\"n\" : null , "
	        "\"f\" : false , "
	        "\"t\" : true , "
	        "\"i\" : 123 , "
	        "\"s\" : \"abc\", "
	        "\"a\" : [ 1, 2, 3 ],"
	        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
	        " } ", status);
	EXPECT_EQ("parse ok", status);
	EXPECT_EQ(json::Object, v.get_type());
	EXPECT_EQ(7, v.get_object_size());
	EXPECT_EQ("n", v.get_object_key(0));
	EXPECT_EQ(json::Null, v.get_object_value(0).get_type());
	EXPECT_EQ("f", v.get_object_key(1));
	EXPECT_EQ(json::False, v.get_object_value(1).get_type());
	EXPECT_EQ("t", v.get_object_key(2));
	EXPECT_EQ(json::True, v.get_object_value(2).get_type());
	EXPECT_EQ("i", v.get_object_key(3));
	EXPECT_EQ(json::Number, v.get_object_value(3).get_type());
	EXPECT_EQ(123.0, v.get_object_value(3).get_number());
	EXPECT_EQ("s", v.get_object_key(4));
	EXPECT_EQ(json::String, v.get_object_value(4).get_type());
	EXPECT_EQ("abc", v.get_object_value(4).get_string());
	EXPECT_EQ("a", v.get_object_key(5));
	EXPECT_EQ(json::Array, v.get_object_value(5).get_type());
	EXPECT_EQ(3, v.get_object_value(5).get_array_size());
	for (int i = 0; i < 3; ++i) {
       yfn::Json e = v.get_object_value(5).get_array_element(i);
	   EXPECT_EQ(json::Number, e.get_type());
	   EXPECT_EQ(i + 1.0, e.get_number());
	}
	EXPECT_EQ("o", v.get_object_key(6));
	{
		yfn::Json o = v.get_object_value(6);
		EXPECT_EQ(json::Object, o.get_type());
		for(int i = 0; i < 3; i++) {
			yfn::Json ov = o.get_object_value(i);
			EXPECT_EQ('1' + i, (o.get_object_key(i))[0]);// (o.get_object_key(i))[0] 表示获得string的第一个字符
			EXPECT_EQ(1, o.get_object_key_length(i));
			EXPECT_EQ(json::Number, ov.get_type());
			EXPECT_EQ(i + 1.0, ov.get_number());
		}
	}
}

#define test_error(error, content)\
	do {\
		yfn::Json v;\
		v.parse(content, status);\
		EXPECT_EQ(error, status);\
		EXPECT_EQ((json::Null), v.get_type());\
	} while(0)


// 测试解析期望值
TEST(TestParseExpectValue, ParseExpectValue)
{
	test_error("parse expect value", "");
	test_error("parse expect value", " ");
}

// 测试解析无效值
TEST(TestParseInvalidValue, ParseInvalidValue)
{
	test_error("parse invalid value", "nul");
	test_error("parse invalid value", "!?");

	test_error("parse invalid value", "+0");
	test_error("parse invalid value", "+1");
	test_error("parse invalid value", ".123"); // 在小数点之前必须有一位数字
	test_error("parse invalid value", "1."); // 在小数点之后必须有一位数字
	test_error("parse invalid value", "INF");
	test_error("parse invalid value", "NAN");
	test_error("parse invalid value", "nan");

#if 1
	test_error("parse invalid value", "[1,]");
	test_error("parse invalid value", "[\"a\", nul]");
#endif
}

// 测试解析非单数根
TEST(TestParseRootNotSingular, ParseRootNotSingular)
{
	test_error("parse root not singular", "null x");
	test_error("parse root not singular", "truead");
	test_error("parse root not singular", "\"dsad\"d");

	test_error("parse root not singular", "0123");
	test_error("parse root not singular", "0x0");
	test_error("parse root not singular", "0x123");
}

// 测试解析数字太大
TEST(TestParseNumberTooBig, ParseNumberTooBig)
{
	test_error("parse number too big", "1e309");
	test_error("parse number too big", "-1e309");
}

// 测试解析缺失引号
TEST(TestParseMissingQuotationMark, ParseMissingQuotationMark)
{
	test_error("parse miss quotation mark", "\"");
	test_error("parse miss quotation mark", "\"abc");
}

// 测试解析无效字符串转义
TEST(TestParseInvalidStringEscape, ParseInvalidStringEscape)
{
#if 1
	test_error("parse invalid string escape", "\"\\v\"");
	test_error("parse invalid string escape", "\"\\'\"");
	test_error("parse invalid string escape", "\"\\0\"");
	test_error("parse invalid string escape", "\"\\x12\"");
#endif
}

// 测试解析无效字符串字符
TEST(TestParseInvalidStringChar, ParseInvalidStringChar)
{
#if 1
	test_error("parse invalid string char", "\"\x01\"");
	test_error("parse invalid string char", "\"\x1F\"");
#endif
}

// 测试解析无效的unicode十六进制
TEST(TestParseInvalidUnicodeHex, ParseInvalidUnicodeHex)
{
	test_error("parse invalid unicode hex", "\"\\u\"");
	test_error("parse invalid unicode hex", "\"\\u0\"");
	test_error("parse invalid unicode hex", "\"\\u01\"");
	test_error("parse invalid unicode hex", "\"\\u012\"");
	test_error("parse invalid unicode hex", "\"\\u/000\"");
	test_error("parse invalid unicode hex", "\"\\uG000\"");
	test_error("parse invalid unicode hex", "\"\\u0/00\"");
	test_error("parse invalid unicode hex", "\"\\u0G00\"");
	test_error("parse invalid unicode hex", "\"\\u0/00\"");
	test_error("parse invalid unicode hex", "\"\\u00G0\"");
	test_error("parse invalid unicode hex", "\"\\u000/\"");
	test_error("parse invalid unicode hex", "\"\\u000G\"");
	test_error("parse invalid unicode hex", "\"\\u 123\"");
}

// 测试解析无效的unicode代理
TEST(TestParseInvalidUnicodeSurrogate, ParseInvalidUnicodeSurrogate)
{
	test_error("parse invalid unicode surrogate", "\"\\uD800\"");
	test_error("parse invalid unicode surrogate", "\"\\uDBFF\"");
	test_error("parse invalid unicode surrogate", "\"\\uD800\\\\\"");
	test_error("parse invalid unicode surrogate", "\"\\uD800\\uDBFF\"");
	test_error("parse invalid unicode surrogate", "\"\\uD800\\uE000\"");
}

// 测试解析缺失逗号或方括号
TEST(TestParseMissCommaOrSquareBracket, ParseMissCommaOrSquareBracket)
{
#if 1
	test_error("parse miss comma or square bracket", "[1");
	test_error("parse miss comma or square bracket", "[1}");
	test_error("parse miss comma or square bracket", "[1 2");
	test_error("parse miss comma or square bracket", "[[]");
#endif
}

// 测试解析缺失key值
TEST(TestParseMissKey, ParseMissKey)
{
	test_error("parse miss key", "{:1,");
	test_error("parse miss key", "{1:1,");
	test_error("parse miss key", "{true:1,");
	test_error("parse miss key", "{false:1,");
	test_error("parse miss key", "{null:1,");
	test_error("parse miss key", "{[]:1,");
	test_error("parse miss key", "{{}:1,");
	test_error("parse miss key", "{\"a\":1,");
}

// 测试解析漏掉逗号或大括号
TEST(TestParseMissCommaOrCurlyBracket, ParseMissCommaOrCurlyBracket)
{
	test_error("parse miss comma or curly bracket", "{\"a\":1");
	test_error("parse miss comma or curly bracket", "{\"a\":1]");
	test_error("parse miss comma or curly bracket", "{\"a\":1 \"b\"");
	test_error("parse miss comma or curly bracket", "{\"a\":{}");
}

// 往返测试：把一个 JSON 解析，然后再生成另一 JSON，逐字符比较两个 JSON 是否一模一样。
// 先将 content 进行解析，然后判断是否解析成功；再然后将 v 生成一个 json 值存储在 status 中，最后比较 content 和 status 是否一样，这样就完成往返测试了
#define test_roundtrip(content)\
	do {\
		yfn::Json v;\
		v.parse(content, status);\
		EXPECT_EQ("parse ok", status);\
		v.stringify(status);\
		EXPECT_EQ(content, status);\
	} while(0)

// 测试序列化数字
TEST(TestStringifyNumber, StringifyNumber)
{
	test_roundtrip("0");
	test_roundtrip("-0");
	test_roundtrip("1");
	test_roundtrip("-1");
	test_roundtrip("1.5");
	test_roundtrip("-1.5");
	test_roundtrip("3.25");
	test_roundtrip("1e+20");
	test_roundtrip("1.234e+20");
	test_roundtrip("1.234e-20");

	test_roundtrip("1.0000000000000002"); /* the smallest number > 1 */
	test_roundtrip("4.9406564584124654e-324"); /* minimum denormal */
	test_roundtrip("-4.9406564584124654e-324");
	test_roundtrip("2.2250738585072009e-308");  /* Max subnormal double */
	test_roundtrip("-2.2250738585072009e-308");
	test_roundtrip("2.2250738585072014e-308");  /* Min normal positive double */
	test_roundtrip("-2.2250738585072014e-308");
	test_roundtrip("1.7976931348623157e+308");  /* Max double */
	test_roundtrip("-1.7976931348623157e+308");
}

// 测试序列化字符串
TEST(TestStringifyString, StringifyString)
{
	test_roundtrip("\"\"");
	test_roundtrip("\"Hello\"");
	test_roundtrip("\"Hello\\nWorld\"");
	test_roundtrip("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	test_roundtrip("\"Hello\\u0000World\"");
}

// 测试序列化数组
TEST(TestStringifyArray, StringifyArray)
{
	test_roundtrip("[]");
	test_roundtrip("[null,false,true,123,\"abc\",[1,2,3]]");
}

// 测试序列化对象
TEST(TestStringifyObject, StringifyObject)
{
	test_roundtrip("{}");
	test_roundtrip("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

// 测试序列化的 null、false、true
TEST(TestStringify, Stringify)
{
	test_roundtrip("null");
	test_roundtrip("true");
	test_roundtrip("false");
}

#define test_equal(json1, json2, equality)\
	do {\
		yfn::Json v1, v2;\
		v1.parse(json1, status);\
		EXPECT_EQ("parse ok", status);\
		v2.parse(json2, status);\
		EXPECT_EQ("parse ok", status);\
		EXPECT_EQ(equality, int(v1 == v2));\
	} while(0)

// 测试是否相等
TEST(TestEqual, Equal)
{
	test_equal("true", "true", 1);
	test_equal("true", "false", 0);
	test_equal("false", "false", 1);
	test_equal("null", "null", 1);
	test_equal("null", "0", 0);
	test_equal("123", "123", 1);
	test_equal("123", "456", 0);
	test_equal("\"abc\"", "\"abc\"", 1);
	test_equal("\"abc\"", "\"abcd\"", 0);
	test_equal("[]", "[]", 1);
	test_equal("[]", "null", 0);
	test_equal("[1,2,3]", "[1,2,3]", 1);
	test_equal("[1,2,3]", "[1,2,3,4]", 0);
	test_equal("[[]]", "[[]]", 1);
	test_equal("{}", "{}", 1);
	test_equal("{}", "null", 0);
	test_equal("{}", "[]", 0);
	test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
	test_equal("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
	test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
	test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
	test_equal("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
	test_equal("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

// 测试是否拷贝
TEST(TestCopy, Copy)
{
	yfn::Json v1, v2;
	v1.parse("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	v2 = v1;
	EXPECT_EQ(1, int(v2 == v1));
}

// 测试是否移动
TEST(TestMove, Move)
{
	yfn::Json v1, v2, v3;
	v1.parse("{\"t\":true, \"f\":false, \"n\":null, \"d\":1.5, \"a\":[1,2,3]}");
	v2 = v1;
	v3 = std::move(v2);
	EXPECT_EQ(json::Null, v2.get_type());
	EXPECT_EQ(1, int(v3 == v1));
}

// 测试是否交换
TEST(TestSwap, Swap)
{
	yfn::Json v1, v2;
	v1.set_string("Hello");
	v2.set_string("World!");
	yfn::swap(v1, v2);
	EXPECT_EQ("World!", v1.get_string());
	EXPECT_EQ("Hello", v2.get_string());
}


// 测试访问null
TEST(TestAccessNull, AccessNull)
{
	yfn::Json v;
	v.set_string("a");
	v.set_null();
	EXPECT_EQ(json::Null, v.get_type());
}

// 测试访问bool
TEST(TestAccessBoolean, AccessBoolean)
{
	yfn::Json v;
	v.set_string("a");
	v.set_boolean(false);
	EXPECT_EQ(json::False, v.get_type());
	
	v.set_string("a");
	v.set_boolean(true);
	EXPECT_EQ(json::True, v.get_type());
}

// 测试访问number
TEST(TestAccessNumber, AccessNumber)
{
	yfn::Json v;
	v.set_string("a");
	v.set_number(1234.5);
	EXPECT_EQ(1234.5, v.get_number());
}

// 测试访问string
TEST(TestAccessString, AccessString)
{
	yfn::Json v;
	v.set_string("");
	EXPECT_EQ("", v.get_string());

	v.set_string("Hello");
	EXPECT_EQ("Hello", v.get_string());
}

// 测试访问array
TEST(TestAccessArray, AccessArray)
{
	yfn::Json a, e;

	for(size_t j = 0; j < 5; j += 5) {
		a.set_array();
		EXPECT_EQ(0, a.get_array_size());
		for(int i = 0; i < 10; ++i){
			e.set_number(i);
			a.pushback_array_element(e);
		}

		EXPECT_EQ(10, a.get_array_size());
		for(int i = 0; i < 10; ++i)
			EXPECT_EQ(static_cast<double>(i), a.get_array_element(i).get_number());
	}

	a.popback_array_element();
	EXPECT_EQ(9, a.get_array_size());
	for (int i = 0; i < 9; ++i)
		EXPECT_EQ(static_cast<double>(i), a.get_array_element(i).get_number());

	a.erase_array_element(4, 0);
	EXPECT_EQ(9, a.get_array_size());
	for (int i = 0; i < 9; ++i)
		EXPECT_EQ(static_cast<double>(i), a.get_array_element(i).get_number());
	
	a.erase_array_element(8,1);
	EXPECT_EQ(8, a.get_array_size());
	for (int i = 0; i < 8; ++i)
		EXPECT_EQ(static_cast<double>(i), a.get_array_element(i).get_number());
	
	a.erase_array_element(0, 2);
	EXPECT_EQ(6, a.get_array_size());
	for (int i = 0; i < 6; ++i)
		EXPECT_EQ(static_cast<double>(i+2), a.get_array_element(i).get_number());
	
	for (int i = 0; i < 2; ++i) {
		e.set_number(i);
		a.insert_array_element(e, i);
	}

	EXPECT_EQ(8, a.get_array_size());
	for (int i = 0; i < 8; ++i)
		EXPECT_EQ(static_cast<double>(i), a.get_array_element(i).get_number());
	
	e.set_string("Hello");
	a.pushback_array_element(e);

	a.clear_array();
	EXPECT_EQ(0, a.get_array_size());
}

// 测试访问object
TEST(TestAccessObject, AccessObject)
{
	yfn::Json o, v;
	for (int j = 0; j <= 5; j += 5) {
		o.set_object();
		EXPECT_EQ(0, o.get_object_size());
		for (int i = 0; i < 10; ++i) {
			std::string key = "a";
			key[0] += i;
			v.set_number(i);
			o.set_object_value(key, v);
		}

		EXPECT_EQ(10, o.get_object_size());
		for (int i = 0; i < 10; ++i) {
			std::string key = "a";
			key[0] += i;// 'a'+1=b，这里就是改变字符
			auto index = o.find_object_index(key);
			EXPECT_EQ(1, static_cast<int> (index >= 0));
			v = o.get_object_value(index);
			EXPECT_EQ(static_cast<double>(i), v.get_number());
		}
	}

	auto index = o.find_object_index("j");
	EXPECT_EQ(1, static_cast<int>(index >= 0));
	o.remove_object_value(index);
	index = o.find_object_index("j");
	EXPECT_EQ(1, static_cast<int>(index < 0));
	EXPECT_EQ(9, o.get_object_size());

	index = o.find_object_index("a");
	EXPECT_EQ(1, static_cast<int>(index >= 0));
	o.remove_object_value(index);
	index = o.find_object_index("a");
	EXPECT_EQ(1, static_cast<int>(index < 0));
	EXPECT_EQ(8, o.get_object_size());

	for (int i = 0; i < 8; i++) {
		std::string key = "a";
		key[0] += i + 1;
		EXPECT_EQ((double)i + 1, o.get_object_value(o.find_object_index(key)).get_number());
	}

	v.set_string("Hello");
	o.set_object_value("World", v);

	index = o.find_object_index("World");
	EXPECT_EQ(1, static_cast<int>(index >= 0));
	v = o.get_object_value(index);
	EXPECT_EQ("Hello", v.get_string());

	o.clear_object();
	EXPECT_EQ(0, o.get_object_size());
}