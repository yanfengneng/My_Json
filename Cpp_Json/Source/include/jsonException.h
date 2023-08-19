#ifndef EXCEPTION_H__
#define EXCEPTION_H__

#include <stdexcept>

namespace miniJson 
{
    /* 异常类 */
    class JsonException final : public std::runtime_error
    {
    public:
        explicit JsonException(const std::string &errMsg) : runtime_error(errMsg) { }
        const char* what() const noexcept override { return runtime_error::what(); }
    };
}

#endif