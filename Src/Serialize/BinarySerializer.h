#pragma once

#include <ostream>
#include <type_traits>
#include <vector>

#include "../Misc/Common.h"
#include "../Misc/Uncopiable.h"

namespace AGZ {

/**
 * @brief 外部序列化设置器
 * 
 * 对任意类型T，若T未实现Serialize，则只需要实现特化类BinarySerializeImplementator<T>中的成员函数：
 *  bool BinarySerializeImplementator<T>::Serialize(BinarySerializer&, const T&)
 * 即可让BinarySerializer::Serialize用该实现来进行T的反序列化
 */
template<typename T>
struct BinarySerializeImplementator { };

/**
 * @brief 二进制序列化器
 */
class BinarySerializer
{
    template<typename T, typename = void>
    struct HasSerialize : std::false_type { };

    template<typename T>
    struct HasSerialize<
        T, std::void_t<decltype(
                std::declval<T>().Serialize(
                    DeclLRef<BinarySerializer>()))>>
        : std::true_type { };

    template<typename T, typename = void>
    struct HasLeftShift : std::false_type { };

    template<typename T>
    struct HasLeftShift<
        T, std::void_t<decltype(
                BinarySerializeImplementator<T>::Serialize(
                    DeclLRef<BinarySerializer>(),
                    std::declval<T>()))>>
        : std::true_type { };

    template<typename T, bool HasLeftShift>
    struct CallLeftShiftImpl
    {
        static bool Call(const T &v, BinarySerializer &serializer)
        {
            return BinarySerializeImplementator<T>::Serialize(serializer, v);
        }
    };

    template<typename T>
    struct CallLeftShiftImpl<T, false>
    {
        static bool Call(const T &v, BinarySerializer &serializer)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            return serializer.Write(&v, sizeof(T));
        }
    };

    template<typename T, bool HasSerialize>
    struct CallSerializeImpl
    {
        static bool Call(const T &v, BinarySerializer &serializer)
        {
            return v.Serialize(serializer);
        }
    };

    template<typename T>
    struct CallSerializeImpl<T, false>
    {
        static bool Call(const T &v, BinarySerializer &serializer)
        {
            return CallLeftShiftImpl<T, HasLeftShift<T>::value>::Call(v, serializer);
        }
    };

    bool ok_ = true;

    virtual bool WriteImpl(const void *ptr, size_t byteSize) = 0;

public:

    virtual ~BinarySerializer() = default;

    /**
     * @brief 向Serializer中写入一定量的二进制数据
     * 
     * @param ptr 指向数据的指针
     * @param byteSize 数据包含多少个字节
     */
    virtual bool Write(const void *ptr, size_t byteSize)
    {
        return (ok_ &= WriteImpl(ptr, byteSize));
    }

    /**
     * @brief 尝试对v进行序列化
     * 
     * - 若v.Serialize(*this)是一个合法表达式，则返回v.Serialize(*this)
     * - 否则，若BinarySerializeImplementator<T>::Serialize(*this, v)是一个合法表达式，则返回其调用结果
     * - 否则，返回Write(&v, sizeof(v))，此时要求T必须是trivially copyable
     */
    template<typename T>
    bool Serialize(const T &v)
    {
        return (ok_ &= CallSerializeImpl<T, HasSerialize<T>::value>::Call(v, *this));
    }

    /**
     * @brief 是否至今为止没有发生过任何序列化错误
     * 
     * @note 若某个时刻发生了一个错误，则后续序列化即使没有发生问题，该方法也会返回false
     */
    bool Ok() const { return ok_; }
};

/**
 * @brief 二进制内存序列化器，即将对象序列化至内存中的BinarySerializer
 */
class BinaryMemorySerializer : public BinarySerializer, public Uncopiable
{
    std::vector<char> dataStream_;

    bool WriteImpl(const void *ptr, size_t byteSize) override
    {
        AGZ_ASSERT(ptr);

        const char *newData = reinterpret_cast<const char*>(ptr);
        size_t oldSize = dataStream_.size();

        try
        {
            dataStream_.resize(dataStream_.size() + byteSize);
        }
        catch(...)
        {
            return false;
        }

        for(size_t i = 0, j = oldSize; i < byteSize; ++i, ++j)
            dataStream_[j] = newData[i];

        return true;
    }

public:

    size_t GetByteSize() const { return dataStream_.size(); }

    const char *GetData() const { return dataStream_.data(); }
};

/**
 * @brief 二进制标准流序列化器，将对象序列化至std::ostream
 */
class BinaryOStreamSerializer : public BinarySerializer, public Uncopiable
{
    std::ostream &os_;

    bool WriteImpl(const void *ptr, size_t byteSize) override
    {
        AGZ_ASSERT(ptr);
        return static_cast<bool>(os_.write((const char*)ptr, byteSize));
    }

public:

    BinaryOStreamSerializer(std::ostream &os) : os_(os) { }
};

}
