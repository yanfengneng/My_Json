#ifndef PARSE_H__
#define PARSE_H__

#include "json.h"
#include "jsonValue.h"

namespace miniJson
{
    /* 对 json 字符串的解析过程 */
    class Parse final
    {
    public:
        Parser(Value &val, const std::string &content);
    private:
        void parse_whitespace() noexcept;
        void parse_value();
        void parse_literal(const char *literal, json::type t);
        void parse_number();
        void parse_string();
        void parse_string_raw(std::string &tmp);
        void parse_hex4(const char* &p, unsigned &u);
        void parse_encode_utf8(std::string &s, unsigned u) const noexcept;
        void parse_array();
        void parse_object();

        JsonValue &val_;
        const char *cur_;
    };
}
#endif