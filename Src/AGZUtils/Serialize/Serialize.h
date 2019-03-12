#pragma once

#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "../Misc/Common.h"

namespace AGZ
{
   
/**
 * @brief 二进制序列化接口
 */
class BinarySerializer
{
    template<typename T, typename = void>
    struct HasSerialize : std::false_type { };
    template<typename T>
    struct HasSerialize<T, std::void_t<decltype(std::declval<T>().Serialize(std::declval<BinarySerializer&>()))>>
        : std::true_type { };

    template<typename T, typename = void>
    struct HasOperatorSerialize : std::false_type { };
    template<typename T>
    struct HasOperatorSerialize<T, std::void_t<decltype(std::declval<BinarySerializer&>() << std::declval<const T&>())>>
        : std::true_type { };

    template<typename T, std::enable_if_t<HasSerialize<T>::value, int> = 0>
    static bool CallSerialize(const T &obj, BinarySerializer &serializer)
    {
        static_assert(std::is_same_v<decltype(obj.Serialize(serializer)), bool>,
                      "obj.Serialize(BinarySerializer&) must return boolean value");
        return obj.Serialize(serializer);
    }

    template<typename T, std::enable_if_t<!HasSerialize<T>::value && HasOperatorSerialize<T>::value, int> = 0>
    static bool CallSerialize(const T &obj, BinarySerializer &serializer)
    {
        (void)(serializer << obj);
        return serializer.Ok();
    }

    template<typename T, std::enable_if_t<!HasSerialize<T>::value &&
                                          !HasOperatorSerialize<T>::value &&
                                          std::is_trivially_copyable_v<T>, int> = 0>
    static bool CallSerialize(const T &obj, BinarySerializer &serializer)
    {
        return serializer.Write(&obj, sizeof(T));
    }

    bool ok_ = true;

protected:

    virtual bool WriteImpl(const void *pData, size_t byteSize) = 0;

public:

    virtual ~BinarySerializer() = default;

    /**
     * @brief 直接写入字节数据
     * 
     * 若调用前未发生序列化错误，则返回写入是否成功；否则写入不会发生且返回false。
     * 
     * @param pData 数据首字节指针
     * @param byteSize 字节数
     * @return 该调用及之前的序列化调用是否都没有发生过错误
     */
    bool Write(const void *pData, size_t byteSize)
    {
        if(ok_)
            ok_ = WriteImpl(pData, byteSize);
        return ok_;
    }

    /**
     * @brief 对给定对象进行二进制序列化
     * 
     * - 若 obj.Serialize(binarySerializer) 是一个合法表达式，则强制要求其结果为bool类型，将调用之并返回其结果。
     * - 否则，若 binarySerializer << obj 是一个合法表达式，则调用之并返回 binarySerializer.Ok() 。
     * - 否则，若obj满足trivially copyable，则直接使用 Write 方法逐字节序列化obj。
     * 
     * 若以上三种情况均不满足，则产生编译错误。
     * 
     * @param obj 被序列化的对象
     * @return 该调用及之前的序列化调用是否都没有发生过错误
     */
    template<typename T>
    bool Serialize(const T &obj)
    {
        if(ok_)
            ok_ = BinarySerializer::CallSerialize<T>(obj, *this);
        return ok_;
    }

    /**
     * @brief 之前的序列化调用是否都没有发生过错误
     */
    bool Ok() const noexcept { return ok_; }
};


/**
 * @brief 二进制内存序列化器
 */
class BinaryMemorySerializer : public BinarySerializer, public Uncopiable
{
    std::vector<char> data_;

protected:

    bool WriteImpl(const void *pData, size_t byteSize) override
    {
        if(!byteSize)
            return true;

        AGZ_ASSERT(pData);

        size_t oldSize = data_.size();

        try
        {
            data_.resize(oldSize + byteSize);
            std::memcpy(&data_[oldSize], pData, byteSize);
            return true;
        }
        catch(...)
        {
            if(data_.size() != oldSize)
                data_.resize(oldSize);
            return false;
        }
    }

public:

    /**
     * @brief 总共积累了多少字节的数据
     */
    size_t GetSize() const noexcept { return data_.size(); }

    /**
     * @brief 目前已经积累的数据指针，仅在下一次序列化调用前保证有效
     */
    const char *GetData() const noexcept { return data_.data(); }
};

/**
 * @brief 二进制标准流序列化器，将对象序列化至std::ostream
 */
class BinaryOStreamSerializer : public BinarySerializer, public Uncopiable
{
    std::ostream &os_;

protected:

    bool WriteImpl(const void *pData, size_t byteSize) override
    {
        if(!byteSize)
            return true;
        AGZ_ASSERT(pData);
        return static_cast<bool>(os_.write(static_cast<const char*>(pData), byteSize));
    }

public:

    explicit BinaryOStreamSerializer(std::ostream &os) noexcept : os_(os) { }
};

/**
 * @brief 二进制反序列化接口
 */
class BinaryDeserializer
{
    template<typename T, typename = void>
    struct HasDeserialize : std::false_type { };
    template<typename T>
    struct HasDeserialize<T, std::void_t<decltype(std::declval<T>().Deserialize(std::declval<BinaryDeserializer&>()))>>
        : std::true_type { };

    template<typename T, typename = void>
    struct HasOperatorDeserialize : std::false_type { };
    template<typename T>
    struct HasOperatorDeserialize<T, std::void_t<decltype(std::declval<BinaryDeserializer&>() >> std::declval<T&>())>>
        : std::true_type { };

    template<typename T, std::enable_if_t<HasDeserialize<T>::value, int> = 0>
    static bool CallDeserialize(T &obj, BinaryDeserializer &deserializer)
    {
        static_assert(std::is_same_v<decltype(obj.Deserialize(deserializer)), bool>,
                      "obj.Deserialize(BinaryDeserializer&) must return boolean value");
        return obj.Deserialize(deserializer);
    }

    template<typename T, std::enable_if_t<!HasDeserialize<T>::value && HasOperatorDeserialize<T>::value, int> = 0>
    static bool CallDeserialize(T &obj, BinaryDeserializer &deserializer)
    {
        (void)(deserializer >> obj);
        return deserializer.Ok();
    }

    template<typename T, std::enable_if_t<!HasDeserialize<T>::value &&
                                          !HasOperatorDeserialize<T>::value &&
                                          std::is_trivially_copyable_v<T>, int> = 0>
    static bool CallDeserialize(T &obj, BinaryDeserializer &deserializer)
    {
        return deserializer.Read(&obj, sizeof(T));
    }

    template<typename T, typename = void>
    struct CallDeserializeFromScratchImpl
    {
        static std::optional<T> Call(BinaryDeserializer &deserializer)
        {
            T ret;
            if(deserializer.Deserialize(ret))
                return std::make_optional(std::move(ret));
            return std::nullopt;
        }
    };

    template<typename T>
    struct CallDeserializeFromScratchImpl<T, std::void_t<decltype((T::Deserialize(std::declval<BinaryDeserializer&>())))>>
    {
        static std::optional<T> Call(BinaryDeserializer &deserializer)
        {
            static_assert(std::is_same_v<decltype(T::Deserialize(deserializer)), std::optional<T>>,
                          "T::Deserialize(deserializer) must return a value of type std::optional<T>");
            return T::Deserialize(deserializer);
        }
    };

    bool ok_ = true;

protected:

    virtual bool ReadImpl(void *pData, size_t byteSize) = 0;

public:

    virtual ~BinaryDeserializer() = default;

    /**
     * @brief 直接读取字节数据
     *
     * 若调用前未发生反序列化错误，则返回读取是否成功；否则读取不会发生且返回false。
     *
     * @param pData 数据首字节指针
     * @param byteSize 字节数
     * @return 该调用及之前的反序列化调用是否都没有发生过错误
     */
    bool Read(void *pData, size_t byteSize)
    {
        if(ok_)
            ok_ = ReadImpl(pData, byteSize);
        return ok_;
    }

    /**
     * @brief 对给定对象进行二进制反序列化
     *
     * - 若 obj.Deserialize(binaryDeserializer) 是一个合法表达式，则强制要求其结果为bool类型，将调用之并返回其结果。
     * - 否则，若 binaryDeserializer >> obj 是一个合法表达式，则调用之并返回 binaryDeserializer.Ok() 。
     * - 否则，若obj满足trivially copyable，则直接使用 Read 方法逐字节反序列化obj。
     *
     * 以上三种情况均不满足时产生编译错误。
     *
     * @param obj 反序列结果的存储对象
     * @return 该调用及之前的反序列化调用是否都没有发生过错误
     */
    template<typename T>
    bool Deserialize(T &obj)
    {
        if(ok_)
            ok_ = BinaryDeserializer::CallDeserialize<T>(obj, *this);
        return ok_;
    }

    /**
     * @brief 二进制反序列化得到一个指定类型的对象
     * 
     * @tparam T 希望获得的对象类型
     * 
     * - 若 T::Deserialize(deserializer) 是一个合法表达式，则强制要求其返回 std::optional<T>，将调用之并返回其结果。
     * - 否则，直接默认构造函数构造一个T对象obj，并用 deserializer.Deserialize(obj) 方法逐字节反序列化。
     * 
     * @return 若该调用及之前的反序列化均未发生错误，则返回包含T对象的std::optional对象；否则返回std::nullopt
     */
    template<typename T>
    std::optional<T> Deserialize()
    {
        if(ok_)
        {
            auto ret = CallDeserializeFromScratchImpl<T>::Call(*this);
            if(!ret)
                ok_ = false;
            return ret;
        }
        return std::nullopt;
    }

    /**
     * @brief 之前的反序列化调用是否都没有发生过错误
     */
    bool Ok() const noexcept { return ok_; }
};

/**
 * @brief 二进制内存反序列化器
 */
class BinaryMemoryDeserializer : public BinaryDeserializer, public Uncopiable
{
    const char *pData_, *pEnd_;

protected:

    bool ReadImpl(void *output, size_t byteSize) override
    {
        if(!byteSize)
            return true;

        if(RemainingByteSize() < byteSize)
            return false;

        auto *outputData = reinterpret_cast<char*>(output);
        while(byteSize-- > 0)
            *outputData++ = *pData_++;

        return true;
    }

public:

    /**
     * @param pData 被反序列化的首字节地址
     * @param byteSize 参与反序列化的总字节数
     */
    BinaryMemoryDeserializer(const void *pData, size_t byteSize)
        : pData_(reinterpret_cast<const char*>(pData)),
          pEnd_(reinterpret_cast<const char*>(pData) + byteSize)
    {

    }

    /**
     * @brief 还剩多少未被反序列化消耗的字节
     */
    size_t RemainingByteSize() const noexcept { return pEnd_ - pData_; }

    /**
     * @brief 是否已经消耗完毕所有的字节
     */
    bool End() const noexcept { return pData_ >= pEnd_; }
};

/**
 * @brief 二进制标准流反序列化器
 */
class BinaryIStreamDeserializer : public BinaryDeserializer, public Uncopiable
{
    std::istream &is_;

protected:

    bool ReadImpl(void *output, size_t byteSize) override
    {
        AGZ_ASSERT(output);
        return static_cast<bool>(is_.read(static_cast<char*>(output), byteSize));
    }

public:

    explicit BinaryIStreamDeserializer(std::istream &is) : is_(is) { }

    /**
     * @brief 是否已经消耗完整个输入流
     */
    bool End() const noexcept { return is_.eof(); }
};

/**
 * @brief 自动实现逐字节拷贝的二进制序列化
 */
#define IMPL_SERIALIZE_WITH_MEMCPY \
    bool Serialize(::AGZ::BinarySerializer &s) const { return s.Write(this, sizeof(*this)); }

 /**
  * @brief 自动实现逐字节拷贝的二进制反序列化
  */
#define IMPL_DESERIALIZE_WITH_MEMCPY \
    bool Deserialize(::AGZ::BinaryDeserializer &s) { return s.Read(this, sizeof(*this)); }

} // namespace AGZ

/**
 * @cond
 */

namespace AGZ::Impl
{
    template<typename...Ts>
    struct DeserializeVariantAux
    {
        template<uint64_t...Is>
        static bool Deserialize(
            uint64_t index, BinaryDeserializer &ds, std::variant<Ts...> &v, std::integer_sequence<uint64_t, Is...>)
        {
            return Aux<Is...>(index, ds, v);
        }

        template<uint64_t Index>
        static bool Aux(
            size_t index, BinaryDeserializer &ds, std::variant<Ts...> &v)
        {
            if(index != Index)
                return false;
            auto tv = ds.Deserialize<std::tuple_element_t<static_cast<size_t>(Index), std::tuple<Ts...>>>();
            if(!tv)
                return false;
            v = std::move(*tv);
            return true;
        }

        template<uint64_t Index, uint64_t SIndex, uint64_t...RestIndex>
        static bool Aux(uint64_t index, BinaryDeserializer &ds, std::variant<Ts...> &v)
        {
            if(Aux<Index>(index, ds, v))
                return true;
            return Aux<SIndex, RestIndex...>(index, ds, v);
        }
    };
    
} // namespace AGZ::Impl

/**
 * @endcond
 */

namespace AGZ
{

/**
 * @brief 二进制序列化std::vector
 * 
 * 先存下一个uint64_t的元素数量，然后逐个将元素序列化
 */
template<typename T>
AGZ::BinarySerializer &operator<<(AGZ::BinarySerializer &s, const std::vector<T> &v)
{
    if(!s.Serialize(static_cast<uint64_t>(v.size())))
        return s;
    for(auto &x : v)
    {
        if(!s.Serialize(x))
            return s;
    }
    return s;
}

/**
 * @brief 二进制反序列化std::vector
 * 
 * 先读取一个uint64_t类型的元素数量，然后逐个将元素反序列化
 */
template<typename T>
AGZ::BinaryDeserializer &operator>>(AGZ::BinaryDeserializer &ds, std::vector<T> &v)
{
    v.clear();
    uint64_t size;
    if(!ds.Deserialize(size))
        return ds;
    v.reserve(static_cast<size_t>(size));

    for(uint64_t i = 0; i < size; ++i)
    {
        auto t = ds.Deserialize<T>();
        if(!t)
            return ds;
        v.push_back(std::move(*t));
    }

    return ds;
}

/**
 * @brief 二进制序列化std::variant
 *
 * 先存下一个uint64_t的类型index，然后序列化内部元素
 */
template<typename...Ts>
AGZ::BinarySerializer &operator<<(AGZ::BinarySerializer &s, const std::variant<Ts...> &v)
{
    if(!s.Serialize(static_cast<uint64_t>(v.index())))
        return s;
    AGZ::MatchVariant(v, [&](auto &x) { s.Serialize(x); });
    return s;
}

/**
 * @brief 二进制反序列化std::variant
 *
 * 先取出一个uint64_t的类型index，然后反序列化内部元素
 */
template<typename...Ts>
AGZ::BinaryDeserializer &operator>>(AGZ::BinaryDeserializer &ds, std::variant<Ts...> &v)
{
    uint64_t index;
    if(!ds.Deserialize(index))
        return ds;
    AGZ::Impl::DeserializeVariantAux<Ts...>::template Deserialize(
        index, ds, v, std::make_integer_sequence<uint64_t, std::tuple_size_v<std::tuple<Ts...>>>());
    return ds;
}

/**
 * @brief 二进制序列化std::basic_string_view，反序列化时应使用对应的std::basic_string
 */
template<typename TChar>
AGZ::BinarySerializer &operator<<(AGZ::BinarySerializer &s, const std::basic_string_view<TChar> &str)
{
    if(!s.Serialize(static_cast<uint64_t>(str.size())))
        return s;
    s.Write(str.data(), str.size() * sizeof(TChar));
    return s;
}

/**
 * @brief 二进制序列化std::basic_string
 */
template<typename TChar>
AGZ::BinarySerializer &operator<<(AGZ::BinarySerializer &s, const std::basic_string<TChar> &str)
{
    return s << std::basic_string_view<TChar>(str);
}

/**
 * @brief 二进制反序列化std::basic_string
 */
template<typename TChar>
AGZ::BinaryDeserializer &operator>>(AGZ::BinaryDeserializer &ds, std::basic_string<TChar> &str)
{
    uint64_t size;
    if(!ds.Deserialize(size))
        return ds;
    str.resize(static_cast<size_t>(size));
    ds.Read(str.data(), static_cast<size_t>(size) * sizeof(TChar));
    return ds;
}

} // namespace AGZ
