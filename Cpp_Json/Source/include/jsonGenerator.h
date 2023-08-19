#ifndef GENERATOR_H__
#define GENERATOR_H__

#include "jsonValue.h"

namespace miniJson
{
    class JsonGenerator final
    {
    public:
        JsonGenerator(const JsonValue& val, std::string &result);
    private:
        void stringify_value(const JsonValue& v);       /* 生成 json 值 */
        void stringify_string(const std::string &str);  /* 生成 json 字符串 */

        std::string &res_;

    };
} // namespace miniJson

#endif