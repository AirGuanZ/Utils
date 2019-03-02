#pragma once

#include <exception>
#include <string>

namespace AGZ
{
    
/**
 * @brief ���ʽ�쳣��ͨ��AGZ_HIERARCHY_TRY��AGZ_HIERARCHY_WRAP��������׳������Լ�¼�׳�·����������HierarchyException��¼����Ϣ
 */
class HierarchyException : public std::exception
{
    std::exception_ptr ptr_;
    std::string what_;

public:

    /**
     * @beief һ����˵��AGZ_HIERARCHY_WRAP���ã��û��Լ�����ʱ ptr ��������
     */
    explicit HierarchyException(std::string what, const std::exception_ptr &ptr = std::exception_ptr())
        : ptr_(ptr), what_(std::move(what))
    {
        
    }

    /**
     * @brief ȡ�ô�����Ϣ
     */
    char const *what() const override
    {
        return what_.c_str();
    }

    /**
     * @brief �������ڣ�ȡ�ø�Exception�׳�·���ϵ�������HierarchyException��¼�Ĵ�����Ϣ
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

#define AGZ_HIERARCHY_TRY try

#define AGZ_HIERARCHY_WRAP(MSG) \
    catch(...) \
    { \
        auto ptr = std::current_exception(); \
        throw ::AGZ::HierarchyException((MSG), ptr); \
    }

} // namespace AGZ
