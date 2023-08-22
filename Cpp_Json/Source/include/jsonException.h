#ifndef JSON_EXCEPTION_H__
#ifndef JSON_EXCEPTION_H__

#include <string>
#include <stdexcept>

namespace yfn
{
    namespace json
    {
        /* 异常类 */
        class Exception final : public std::logic_error
        {
        public:
            Exception(const std::string& errMsg) : logic_error(errMsg) { }
        };
    }
}

#endif