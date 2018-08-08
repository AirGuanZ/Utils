#pragma once

#include <algorithm>
#include <type_traits>
#include <vector>

AGZ_NS_BEG(AGZ::StrImpl)

template<typename T, typename CS,
         std::enable_if_t<std::is_intergal_v<T>, int> = 0>
String<CS> Int2Str(T v, unsigned int base)
{
    AGZ_ASSERT(2 <= base && base <= 36);
    std::vector<typename CS::CodeUnit> cus;

    [[maybe_unused]]
    bool neg = false;

    // Handle negative sign
    if constexpr(std::is_signed_v<T>)
    {
        // Corner case ignored: v == numeric_limits<T>::min()
        if(v < 0)
        {
            neg = true;
            v = ~v + 1;
        }
    }

    if(!v)
        cus.push_back('0');
    else
    {
        while(v)
        {
            static const char chs[36] =
            {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                'U', 'V', 'W', 'X', 'Y', 'Z'
            };
            auto d = v % base;
            v      = v / base;
            cus.push_back(chs[d]);
        }
    }

    if constexpr(std::is_signed_v<T>)
    {
        if(neg)
            cus.push_back('-');
    }

    std::reverse(std::begin(cus), std::end(cus));
    return String<CS>(cus.data(), cus.size());
}

AGZ_NS_END(AGZ::StrImpl)
