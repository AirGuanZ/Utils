#pragma once

#include "../../Misc/Common.h"

AGZ_NS_BEG(AGZ)

namespace CharsetAux
{
    template<typename It>
    struct CodeUnitsFromCodePointIteratorImpl
    {
        static auto Eval(const It &it)
        {
            return it.CodeUnitsFromCodePointIterator();
        }
    };

    template<typename CU>
    struct CodeUnitsFromCodePointIteratorImpl<CU*>
    {
        static auto Eval(const CU *it)
        {
            return std::pair<const CU*, const CU*>{ it, it + 1 };
        }
    };
}

template<typename Core>
class Charset : public Core
{
public:

    using Iterator  = typename Core::Iterator;
    using CodePoint = typename Core::CodePoint;
    using CodeUnit  = typename Core::CodeUnit;

    static size_t Length(const CodeUnit *cu)
    {
        size_t ret = 0;
        while(*cu++)
            ++ret;
        return ret;
    }

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
    static size_t TranslateTo(const CodeUnit *beg, const CodeUnit *end,
                              typename OCS::CodeUnit *dst, size_t buf_size)
    {
        typename OCS::CodeUnit tBuf[OCS::MaxCUInCP];
        size_t ret = 0;
        while(beg < end && buf_size)
        {
            CodePoint cp;
            adv = CU2CP(beg, &cp);
            if(!adv)
                return 0;
            size_t tsize = OCS::CP2CU(To<OCS>(cp), tBuf);
            if(tsize > buf_size)
                return ret;
            ret += tsize;
            beg += adv;
            for(size_t i = 0; i < tsize; ++i)
                dst[i] = tBuf[i];
            dst += tsize;
        }
        return ret;
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

    static bool IsSpace(CodeUnit cu)
    {
        return cu == ' ' || cu == '\t' ||
               cu == '\n' || cu == '\r';
    }

    static std::pair<const CodeUnit*, const CodeUnit*>
    CodeUnitsFromCodePointIterator(const typename Core::Iterator &it)
    {
        return CharsetAux::CodeUnitsFromCodePointIteratorImpl<
                                    typename Core::Iterator>(it);
    }
};

AGZ_NS_END(AGZ)
