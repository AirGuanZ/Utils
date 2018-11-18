#pragma once

#include <stdexcept>
#include <string>

namespace AGZ {

/**
 * @brief AGZ Utils自定义的所有异常的基类
 */
class Exception : public std::runtime_error
{
public:
    explicit Exception(const std::string &err) : runtime_error(err) { }
};

#define AGZ_NEW_EXCEPTION(NAME) \
    class NAME : public Exception \
    { \
    public: \
        explicit NAME(const std::string &err) : Exception(err) { } \
    }

AGZ_NEW_EXCEPTION(CharsetException);
AGZ_NEW_EXCEPTION(ArgumentException);
AGZ_NEW_EXCEPTION(OSException);
AGZ_NEW_EXCEPTION(FileException);
AGZ_NEW_EXCEPTION(UnreachableException);

} // namespace AGZ
