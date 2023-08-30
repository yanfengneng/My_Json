#ifndef PARSE_H__
#ifndef PARSE_H__

#include "json.h"
#include "jsonValue.h"

namespace yfn
{
    namespace json
    {
        /* 对 json 字符串的解析过程 */
        class Parser final
        {
        public:
            Parser(Value &val, const std::string &content);
        private:
            /* 处理空白 */
            void parse_whitespace() noexcept;
            /* 解析 json 值 */
            void parse_value();
            /* 合并 false、true、null 的解析函数 */
            void parse_literal(const char *literal, json::type t);
            /* 解析数字 */
            void parse_number();
            /* 将之前解析字符串的函数拆分为两部分，是为了在解析 json 对象的 key 值时，不使用 lept_value 存储键，因为这样会浪费其中的 type 这个无用字段 */
            void parse_string();
            /* 解析 字符串 */
            void parse_string_raw(std::string &tmp);
            /* 读4位16进制数字 */
            void parse_hex4(const char* &p, unsigned &u);
            /* 把码点编码成 utf-8 */
            void parse_encode_utf8(std::string &s, unsigned u) const noexcept;
            /* 解析数组 */
            void parse_array();
            /* 解析对象 */
            void parse_object();

            Value &val_;
            const char *cur_;
        };
    } // namespace json
    
}

#endif