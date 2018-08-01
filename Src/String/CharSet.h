#pragma once

#include "../Misc/Common.h"

AGZ_NS_BEG(AGZ)

template<typename Core>
class Charset : public Core
{
public:

    using Iterator  = typename Core::Iterator;
    using CodePoint = typename Core::CodePoint;
    using CodeUnit  = typename Core::CodeUnit;

    static bool Check(const CodeUnit *beg, size_t n)
    {
        CodePoint cp;
        while(n)
        {
            size_t s = Core::CU2CP(beg, &cp, n);
            if(!s)
                return false;
            n -= s;
            beg += s;
        }
        return true;
    }

    template<typename OCS>
    static CodePoint From(typename OCS::CodePoint ocp)
    {
        return Core::FromUnicode(OCS::ToUnicode(ocp));
    }

    template<typename OCS>
    static typename OCS::CodePoint To(CodePoint cp)
    {
        return OCS::FromUnicode(Core::ToUnicode(cp));
    }
};

AGZ_NS_END(AGZ)
