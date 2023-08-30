#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include "jsonException.h"
#include "parser.h"

namespace yfn
{
    namespace json
    {
        /* 跳过相等的字符 */
        inline void expect(const char * &c, char ch)
        {
            assert(*c == ch);
            ++c;
        }

        Parser::Parser(Value &val, const std::string &content)
            : val_(val), cur_(content.c_str())
        {
            // 先设置 Value 的类型为 null
            val_.set_type(json::Null);
            // 去掉 Value 前后的空白，若 json 在一个值之后，空白之后还有其他字符的话，说明该 json 值是不合法的。
            parse_whitespace();
            parse_value();
            parse_whitespace();
            if(*cur_ != '\0'){
                val_.set_type(json::Null);
                throw(Exception("parse root not singular"));
            }
        }

        /* 处理空白 */
        void Parser::parse_whitespace() noexcept
        {
            /* 过滤掉 json 字符串中的空白，即空格符、制表符、换行符、回车符 */
            while (*cur_ == ' ' || *cur_ == '\t' || *cur_ == '\n' || *cur_ == '\r')
                ++cur_;
        }

        /* 解析 json 值 */
        void Parser::parse_value()
        {
            switch (*cur_)
            {
            case 'n': parse_literal("null", json::Null); return;
            case 't': parse_literal("true",json::True); return;
            case 'f': parse_literal("false", json::False); return;
            default: parse_number(); return;
            case '\"': parse_string(); return;
            case '[': parse_array(); return;
            case '{': parse_object();return;
            case '\0': throw(Exception("parse expect value"));
            }
        }

        /* 合并 false、true、null 的解析函数 */
        void Parser::parse_literal(const char *literal, json::type t);
        {
            expect(cur_, literal[0]);
            size_t i;
            for(i = 0; literal[i+1]; i++){// 直到 literal[i+1] 为 '\0'，循环结束
                if (cur_[i] != literal[i+1])// 解析失败，抛出异常
                    throw (Exception("parse invalid value"));
            }
            // 解析成功，将 cur_ 右移 i 位，然后设置 val_ 的类型为 t
            cur_ += i;
            val_.set_type(t);
        }

        /* 解析数字 */
        void Parser::parse_number();
        {
            const char *p = cur_;
            // 处理负号
            if(*p == '-') p++;

            // 处理整数部分，分为两种合法情况：一种是单个 0，另一种是一个 1~9 再加上任意数量的 digit。
            if(*p == '0') p++;
            else {
                if(!isdigit(*p))throw (Exception("parse invalid value"));
                while(isdigit(*++p));
            }

            // 处理小数部分：小数点后面第一个数不是数字，则抛出异常，然后再处理连续的数字
            if(*p == '.'){
                if(!isdigit(*++p))throw (Exception("parse invalid value"));
                while(isdigit(*++p));
            }

            // 处理指数部分：需要处理指数的符号，符号之后的第一个字符不是数字，则抛出异常；然后再处理连续的数字
            if(*p == 'e' || *p == 'E'){
                ++p;
                if(*p == '+' || *p == '-') ++p;
                if(!isdigit(*p))throw (Exception("parse invalid value"));
                while(isdigit(*++p));
            }

            errno = 0;
            // 将 json 的十进制数字转换为 double 型的二进制数字
            double v = strtod(cur_, NULL);
            // 如果转换出来的数字过大，则抛出异常
            if (errno == ERANGE && (v == HUGE_VAL || v == -HUGE_VAL))
                throw (Exception("parse number too big"));

            // 最后设置 Value 为数字，然后更新 cur_ 的位置
            val_.set_number(v);
            cur_ = p;
        }

        /* 将之前解析字符串的函数拆分为两部分，是为了在解析 json 对象的 key 值时，不使用 lept_value 存储键，因为这样会浪费其中的 type 这个无用字段 */
        void Parser::parse_string();
        {
            std::string s;
            // 用临时值 s 来保存解析出来的字符串，然后将 s 赋值为 Value
            parse_string_raw(s);
            val_.set_string(s);
        }

        /* 解析字符串 */
        void Parser::parse_string_raw(std::string &tmp)
        {
            except(cur_, '\"');// 跳过字符串的第一个引号
            const char *p = cur_;
            unsigned u = 0, u2 = 0;
            while (*p != '\"')// 直到解析到字符串结尾，也就是第二个引号
            {
                // 字符串的结尾不是双引号，说明该字符串缺少引号，抛出异常即可
                if (*p == '\0')
                    throw (Exception("parse miss quotation mark"));
                // 处理 9 种转义字符：当前字符是'\'，然后跳到下一个字符
                if (*p == '\\' && ++p)
                {
                    case '\"': tmp += '\"'; break;
                    case '\\': tmp += '\\'; break;
                    case '/' : tmp += '/' ; break;
                    case 'b' : tmp += 'b' ; break;
                    case 'f' : tmp += 'f' ; break;
                    case 'n' : tmp += 'n' ; break;
                    case 'r' : tmp += 'r' ; break;
                    case 't' : tmp += 't' ; break;
                    case 'u' :
                        // 遇到\u转义时，调用parse_hex4()来解析4位十六进制数字
                        parse_hex4(p, u);
                        if (u >= 0xD800 && u <= 0xDBFF) 
                        {
                            if (*p++ != '\\')
                                throw (Exception("parse invalid unicode surrogate"));
                            if (*p++ != 'u')
                                throw (Exception("parse invalid unicode surrogate"));
                            parse_hex4(p, u2);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                    throw(Exception("parse invalid unicode surrogate"));
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        // 把码点编码成 utf-8，写进缓冲区
                        parse_encode_utf8(tmp, u);
                        break;  
                    default : throw (Exception("parse invalid string escape!"));
                }
                else if ((unsigned char) *p < 0x20){
                    throw (Exception("parse invalid string char"));
                }
                else tmp += *p++;
            }
            // 更新当前字符串的位置
            cur_ = ++p;
        }

        /* 读4位16进制数字 */
        void Parser::parse_hex4(const char* &p, unsigned &u)
        {
            u = 0;
            for (size_t i = 0; i < 4; ++i)
            {
                char ch = *p++;
                u <<= 4;
                if (isdigit(ch))
                    u |= ch - '0';
                else if (ch >= 'A' && ch <= 'F')
                    u |= ch - ('A' - 10);
                else if (ch >= 'a' && ch <= 'f')
                    u |= ch - ('a' - 10);
                else throw (Exception('parse invalid unicode hex'));
            }
        }

        /* 把码点编码成 utf-8 */
        void Parser::parse_encode_utf8(std::string &s, unsigned u) const noexcept
        {
            if (u <= 0x7F)
                str += static_cast<char> (u & 0xFF);
            else if (u <= 0x7FF) {
                str += static_cast<char> (0xC0 | ((u >> 6) & 0xFF));
                str += static_cast<char> (0x80 | ( u	   & 0x3F));
            }
            else if (u <= 0xFFFF) {
                str += static_cast<char> (0xE0 | ((u >> 12) & 0xFF));
                str += static_cast<char> (0x80 | ((u >>  6) & 0x3F));
                str += static_cast<char> (0x80 | ( u        & 0x3F));
            }
            else {
                assert(u <= 0x10FFFF);
                str += static_cast<char> (0xF0 | ((u >> 18) & 0xFF));
                str += static_cast<char> (0x80 | ((u >> 12) & 0x3F));
                str += static_cast<char> (0x80 | ((u >>  6) & 0x3F));
                str += static_cast<char> (0x80 | ( u        & 0x3F));
            }
        }

        /* 解析数组 */
        void Parser::parse_array();
        {
            expect(cur_, '[');// 处理数字的左括号，然后将当前字符的位置右移一位
            parse_whitespace();// 第一个解析空白：在左括号之后解析空白
            std::vector<Value> tmp;
            if (*cur_ == ']'){// 遇到数组的右括号，然后将当前字符位置右移一位，并将 Value 设置为数组 tmp
                ++cur_;
                val_.set_array(tmp);
                return;
            }
            for(;;)
            {
                // 先解析 json 值，如解析出现异常，则将 Value 设置为 null，然后抛出异常
                try {
                    parse_value();
                } catch(Exception) {
                    val_.set_type(json::Null);
                    throw;
                }
                // 将解析出来的值加入到 tmp 后面
                tmp.push_back(val_);
                parse_whitespace();// 第二个解析空白：在逗号之后处理空白

                // 值之后若为逗号，将当前字符的位置右移一位，然后处理逗号之后的空白
                if (*cur_ == ',') {
                    ++cur_;
                    parse_whitespace();// 第三个解析空白：在逗号之后处理空白
                }

                // 值之后若为右括号，则将当前字符的位置右移一位，然后将 val_ 设置为数组 tmp
                else if(*cur_ == ']') {
                    ++cur_;
                    val_.set_array(tmp);
                    return;
                }

                // 若遇到解析失败，则直接抛出异常
                else {
                    val_.set_type(json::Null);
                    throw(Exception("parse miss comma or square bracket"));
                }
            }
        }

        /* 解析对象 */
        void Parser::parse_object();
        {
            expect(cur_, '{'); // 先跳过左花括号
            parse_whitespace(); // 第一个解析空白：在左花括号之后处理空白
            std::vector<std::pair<std::string, Value>> tmp;
            std::string key;

            // 遇到对象的右花括号，然后将当前字符的位置右移一位，然后 val_ 设置为对象 tmp
            if (*cur_ == '}') {
                ++cur_;
                val_.set_object(tmp);
                return;
            }

            for(;;) {
                /* 1、解析 key 值：若解析失败，则抛出异常 */
                if (*cur_ != '\"') throw(Exception("parse miss key"));
                try {
                    parse_string_raw(key);
                } catch (Exception) {
                    throw(Exception("parse miss key"));
                }
                
                /* 2、解析"_:_"，冒号前后可有空白字符 */
                parse_whitespace();// 第二个解析空白：处理冒号之前的所有空白
                if (*cur_++ != ':') throw (Exception("parse miss colon"));
                parse_whitespace();// 第三个解析空白：处理冒号之后的所有空白

                /* 3、解析冒号之后的值 */
                try {
                    parse_value();
                } catch(Exception) {
                    val_.set_type(json::Null);
                    throw;
                }

                // 把解析到的值存入 tmp 中，然后将解析到的 val_ 设置为 null，并将 key 进行清空
                tmp.push_back(make_pair(key, val_));
                val_.set_type(json::Null);
                key.clear();

                /* 4、解析 "_,_" 或 "_}" */
                parse_whitespace();// 第四个解析空白：处理逗号或右花括号之前的空白
                if (*cur_ == ',') {// 处理逗号
                    ++cur_;
                    parse_whitespace();// 第五个解析空白：处理逗号之后的空白
                }
                else if (*cur_ == '}'){// 处理右花括号：将当前字符的位置右移一位，并设置 val_ 为对象 tmp
                    ++cur_;
                    val_.set_object(tmp);
                    return;
                }
                else { // 若解析失败，则将 val_ 的类型设置为 null，并抛出异常
                    val_.set_type(json::Null);
                    throw(Exception("parse miss comma or curly bracket"));
                }
            }
        }
    } // namespace json  
}