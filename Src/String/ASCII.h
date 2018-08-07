#pragma once

#include <type_traits>

#include "../Misc/Common.h"
#include "Charset.h"

AGZ_NS_BEG(AGZ)

template<typename T>
class ASCIICore
{
public:

    using Iterator  = const T*;
    using CodePoint = char;
    using CodeUnit  = T;

    static std::string Name() { return "ASCII"; }

    static constexpr size_t MaxCUInCP = 1;

    static size_t CUInCP(CodePoint cp) { return 1; }

    static size_t CP2CU(CodePoint cp, CodeUnit *cu)
    {
        cu[0] = static_cast<CodeUnit>(cp);
        return 1;
    }

    static size_t CU2CP(const CodeUnit *cu, CodePoint *cp)
    {
        *cp = static_cast<CodePoint>(*cu);
        return 1;
    }

    static char32_t ToUnicode(CodePoint cp) { return cp; }
    static CodePoint FromUnicode(char32_t cp)
    {
        if(cp > 127)
            return static_cast<CodePoint>('?');
        return static_cast<CodePoint>(cp);
    }
};

template<typename T = char>
using ASCII = Charset<ASCIICore<T>>;

AGZ_NS_END(AGZ)
