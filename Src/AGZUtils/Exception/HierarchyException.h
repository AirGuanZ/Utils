#pragma once

#include <exception>
#include <string>

namespace AGZ
{
    
template<typename TOutputIterator>
void ExtractHierarchyExceptions(const std::exception &e, TOutputIterator outputIterator)
{
    *outputIterator++ = e.what();
    try
    {
        std::rethrow_if_nested(e);
    }
    catch(const std::exception &e2)
    {
        ExtractHierarchyExceptions(e2, outputIterator);
    }
    catch(...)
    {
        *outputIterator++ = std::string("an unknown exception was thrown");
    }
}

#define AGZ_HIERARCHY_TRY try {

#define AGZ_HIERARCHY_WRAP(MSG) \
    } \
    catch(...) \
    { \
        std::throw_with_nested(std::runtime_error(MSG)); \
    }

} // namespace AGZ
