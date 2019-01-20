#pragma once

#include <vector>

#include "../../Misc/TypeOpr.h"
#include "../String/String.h"
#include "../String/StrAlgo.h"

namespace AGZ
{
    
AGZ_NEW_EXCEPTION(ScannerStringException);

namespace ScannerImpl
{
    
    template<typename CS>
    class TScanner
    {
        struct Char { char32_t cp; };
        struct GetInt32_t { };
        struct GetUInt32_t { };
        struct GetUnknown { };
        struct EndOfInput { };

        using Unit = TypeOpr::Variant<
            Char,
            GetInt32_t, GetUInt32_t,
            GetUnknown,
            EndOfInput>;

        std::vector<Unit> cpr_;

        // parse第一个unsigned int并将cur置于第一个非digit的位置上
        static unsigned int ParseUInt(typename CodePointRange<CS>::Iterator &cur, typename CodePointRange<CS>::Iterator end)
        {
            if(cur == end || !StrAlgo::IsUnicodeDigit(*cur))
                throw ScannerStringException("Invalid scanner format: invalid integer literal");
            unsigned int ret = 0;
            for(;;)
            {
                char32_t c = *cur;
                if(StrAlgo::IsUnicodeDigit(c))
                    ret = 10 * ret + (c - '0');
                else
                    return ret;
                if(++cur == end)
                    return ret;
            }
        }

        static void InitVoidPtrArray(void **) noexcept { }

        template<typename FRcv, typename...Rcvs>
        static void InitVoidPtrArray(void **vRcvs, FRcv &frcv, Rcvs&...rcvs) noexcept
        {
            vRcvs[0] = &frcv;
            InitVoidPtrArray(vRcvs + 1, rcvs...);
        }
        
    public:

        explicit TScanner(const Str8 &fmt)
        {
            auto cpr = fmt.CodePoints();
            for(auto it = cpr.begin(), end = cpr.end(); it != end; ++it)
            {
                char32_t c = *it;

                if(c != '{')
                {
                    cpr_.emplace_back(Char{ c });
                    continue;
                }

                if(++it == end)
                    throw ScannerStringException("Invalid scanner format: '}' expected");

                auto fc = *it;
                switch(fc)
                {
                case '{':
                    cpr_.emplace_back(Char{ '{' });
                    break;
                case '}':
                    cpr_.emplace_back(GetUnknown{});
                    break;
                case 'i':
                {
                    auto num = ParseUInt(++it, end);
                    if(num == 32)
                        cpr_.emplace_back(GetInt32_t{});
                    if(it == end || *it != '}')
                        throw ScannerStringException("Inalid scanner format: '}' expected");
                    break;
                }
                case 'u':
                {
                    auto num = ParseUInt(++it, end);
                    if(num == 32)
                        cpr_.emplace_back(GetUInt32_t{});
                    if(it == end || *it != '}')
                        throw ScannerStringException("Inalid scanner format: '}' expected");
                    break;
                }
                default:
                    throw ScannerStringException("Invalid scanner format: unknown character between { }");
                }
            }
        }

        template<typename...Rcvs>
        bool Scan(const String<CS> &, Rcvs&...) const
        {
            // TODO
            return true;
        }
    };

} // namespace ScannerImpl

} // namespace AGZ
