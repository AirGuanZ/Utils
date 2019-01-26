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

#define AGZ_NEW_EXCEPTION(NAME, BASE_NAME) \
    class NAME : public BASE_NAME \
    { \
    public: \
		using BASE_NAME::BASE_NAME; \
    }

AGZ_NEW_EXCEPTION(CharsetException,     Exception);
AGZ_NEW_EXCEPTION(ArgumentException,    Exception);
AGZ_NEW_EXCEPTION(OSException,          Exception);
AGZ_NEW_EXCEPTION(FileException,        Exception);
AGZ_NEW_EXCEPTION(UnreachableException, Exception);

} // namespace AGZ
