#ifndef JSON_GENERATOR_H__
#define JSON_GENERATOR_H__

#include "jsonValue.h"

namespace yfn
{
    namespace json
    {
        /* json 生成器 */
        class Generator final
        {
        public:
            Generator(const Value& val, std::string& result);
        private:
            void stringify_value(const Value &v);
            void stringify_string(const std::string &str);

            std::string &res_;
        };
    }
} // namespace yfn

#endif