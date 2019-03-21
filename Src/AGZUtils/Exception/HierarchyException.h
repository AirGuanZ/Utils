#pragma once

#include <exception>
#include <string>

namespace AGZ
{
    
/**
 * @brief 层次式异常，通过AGZ_HIERARCHY_TRY和AGZ_HIERARCHY_WRAP捕获和再抛出，可以记录抛出路径上所有由HierarchyException记录的消息
 */
class HierarchyException : public std::exception
{
    std::exception_ptr ptr_;
    std::string what_;

public:

    /**
     * @beief 一般来说由AGZ_HIERARCHY_WRAP调用，用户自己调用时 ptr 参数留空
     */
    explicit HierarchyException(std::string what, const std::exception_ptr &ptr = std::exception_ptr())
        : ptr_(ptr), what_(std::move(what))
    {
        
    }

    /**
     * @brief 取得错误消息
     */
    char const *what() const noexcept override
    {
        return what_.c_str();
    }

    /**
     * @brief 由外向内，取得该Exception抛出路径上的所有由HierarchyException记录的错误消息
     */
    template<typename OutputIterator>
    void GetAllMessages(OutputIterator outputIterator) const
    {
        *outputIterator++ = what_;

        try
        {
            throw ptr_;
        }
        catch(...)
        {
            auto p = std::current_exception();
            while(p)
            {
                try
                {
                    std::rethrow_exception(p);
                }
                catch(const HierarchyException &e)
                {
                    *outputIterator++ = e.what_;
                    p = e.ptr_;
                }
                catch(const std::exception &e)
                {
                    *outputIterator++ = std::string(e.what());
                    p = std::exception_ptr();
                }
                catch(...)
                {
                    *outputIterator++ = std::string("an unknown exception was thrown");
                    p = std::exception_ptr();
                }
            }
        }
    }
};

#define AGZ_HIERARCHY_TRY try {

#define AGZ_HIERARCHY_WRAP(MSG) \
    } \
    catch(...) \
    { \
        auto ptr = std::current_exception(); \
        throw ::AGZ::HierarchyException((MSG), ptr); \
    }

} // namespace AGZ
