#pragma once

#include <vector>

#include "../Misc/TypeOpr.h"
#include "BinaryDeserializer.h"
#include "BinarySerializer.h"

#define IMPL_MEMCPY_SERIALIZE \
    bool Serialize(::AGZ::BinarySerializer &s) const { return s.Write(this, sizeof(*this)); }
#define IMPL_MEMCPY_DESERIALIZE \
    bool Deserialize(::AGZ::BinaryDeserializer &d) { return d.Read(this, sizeof(*this)); }

namespace AGZ {

template<typename T>
struct BinarySerializeImplementator<std::vector<T>>
{
    static bool Serialize(BinarySerializer &serializer, const std::vector<T> &v)
    {
        if(!serializer.Serialize(v.size()))
            return false;
        for(auto &x : v)
        {
            if(!serializer.Serialize(x))
                return false;
        }
        return true;
    }
};

template<typename T>
struct BinaryDeserializeImplementator<std::vector<T>>
{
    static bool Deserialize(BinaryDeserializer &deserializer, std::vector<T> &v)
    {
        v.clear();
        typename std::vector<T>::size_type size;
        if(!deserializer.Deserialize(size))
            return false;
        v.reserve(size);
        for(size_t i = 0; i < size; ++i)
        {
            T t;
            if(!deserializer.Deserialize(t))
                return false;
            v.push_back(std::move(t));
        }
        return true;
    }
};

template<typename...Ts>
struct BinarySerializeImplementator<TypeOpr::Variant<Ts...>>
{
    static bool Serialize(BinarySerializer &serializer, const TypeOpr::Variant<Ts...> &v)
    {
        if(!serializer.Serialize(v.index()))
            return false;
        return TypeOpr::MatchVar(v, [&](auto &x)
        {
            return serializer.Serialize(x);
        });
    }
};

template<typename...Ts>
struct BinaryDeserializeImplementator<TypeOpr::Variant<Ts...>>
{
    static bool Deserialize(BinaryDeserializer &deserializer, TypeOpr::Variant<Ts...> &v)
    {
        size_t index;
        if(!deserializer.Deserialize(index))
            return false;
        return DeserializeAux(
            index, deserializer, v, std::make_integer_sequence<size_t, TypeOpr::TypeListLength_v<Ts...>>());
    }

private:

    template<size_t...Is>
    static bool DeserializeAux(
        size_t index, BinaryDeserializer &deserializer, TypeOpr::Variant<Ts...> &v,
        std::integer_sequence<size_t, Is...>)
    {
        return DeserializeAux2<Is...>(index, deserializer, v);
    }

    template<size_t Index>
    static bool DeserializeAux2(
        size_t index, BinaryDeserializer &deserializer, TypeOpr::Variant<Ts...> &v)
    {
        if(index != Index)
            return false;
        TypeOpr::SelectInTypeList_t<Index, Ts...> tv;
        if(!deserializer.Deserialize(tv))
            return false;
        v = std::move(tv);
        return true;
    }

    template<size_t Index, size_t SIndex, size_t...RestIndex>
    static bool DeserializeAux2(size_t index, BinaryDeserializer &deserializer, TypeOpr::Variant<Ts...> &v)
    {
        if(DeserializeAux2<Index>(index, deserializer, v))
            return true;
        return DeserializeAux2<SIndex, RestIndex...>(index, deserializer, v);
    }
};

}
