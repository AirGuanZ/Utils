#pragma once

#include <istream>
#include <type_traits>

#include "../Misc/Common.h"
#include "../Misc/Uncopiable.h"

namespace AGZ {

/**
 * @brief 外部反序列化设置器
 * 
 * 对任意类型T，若T未实现Deserialize，则只需要实现特化类BinaryDeserializeImplementator<T>中的成员函数：
 *  bool BinaryDeserializeImplementator<T>::Deserialize(BinaryDeserializer&, T&)
 * 即可让BinaryDeserializer::Deserialize用该实现来进行T的反序列化
 */
template<typename T> struct BinaryDeserializeImplementator { };

class BinaryDeserializer
{
    template<typename T, typename = void>
    struct HasDeserialize : std::false_type { };

    template<typename T>
    struct HasDeserialize<
        T, std::void_t<decltype(
            std::declval<T>().Deserialize(
                *((BinaryDeserializer*)(nullptr))))>>
        : std::true_type { };
    
    template<typename T, typename = void>
    struct HasRightShift : std::false_type { };

    template<typename T>
    struct HasRightShift<
        T, std::void_t<decltype(
            BinaryDeserializeImplementator<T>::Deserialize(
                *((BinaryDeserializer*)(nullptr)),
                *((remove_rcv_t<T>*)(nullptr))))>>
        : std::true_type { };

    template<typename T, bool HasRightShift>
    struct CallRightShiftImpl
    {
        static bool Call(T &v, BinaryDeserializer &deserializer)
        {
            return BinaryDeserializeImplementator<T>::Deserialize(deserializer, v);
        }
    };

    template<typename T>
    struct CallRightShiftImpl<T, false>
    {
        static bool Call(T &v, BinaryDeserializer &deserializer)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            return deserializer.Read(&v, sizeof(T));
        }
    };

    template<typename T, bool HasDeserialize>
    struct CallDeserializeImpl
    {
        static bool Call(T &v, BinaryDeserializer &deserializer)
        {
            return v.Deserialize(deserializer);
        }
    };

    template<typename T>
    struct CallDeserializeImpl<T, false>
    {
        static bool Call(T &v, BinaryDeserializer &deserializer)
        {
            return CallRightShiftImpl<T, HasRightShift<T>::value>::Call(v, deserializer);
        }
    };

    bool ok_ = true;

    virtual bool ReadImpl(void *output, size_t byteSize) = 0;

public:

    /**
     * @brief 从Serializer中读取一定量的二进制数据
     * 
     * @param output 数据输出至该指针指向的区域
     * @param byteSize 要读取的字节数
     */
    bool Read(void *output, size_t byteSize)
    {
        return (ok_ = ReadImpl(output, byteSize));
    }

    /**
     * @brief 尝试对T进行反序列化并存入v中
     * 
     * - 若v.Deserialize(*this)是一个合法表达式，则返回v.Deserialize(*this)
     * - 否则，若BinaryDeserializeImplementator<T>::Deserialize(*this, v)是一个合法表达式，则返回其调用结果
     * - 否则，返回Read(&v, sizeof(v))，此时要求T必须是trivially copyable
     */
    template<typename T>
    bool Deserialize(T &v)
    {
        return (ok_ &= CallDeserializeImpl<T, HasDeserialize<T>::value>::Call(v, *this));
    }

    /**
     * @brief 是否至今为止没有发生过任何反序列化错误
     */
    bool Ok() const { return ok_; }

    /**
     * @brief 是否已经到达字节流的末端
     */
    virtual bool End() = 0;
};

/**
 * @brief 二进制内存反序列化器，即将从内存反序列化得到对象的BinaryDeserializer
 */
class BinaryMemoryDeserializer : public BinaryDeserializer, public AGZ::Uncopiable
{
    const char *pData_, *pEnd_;

    bool ReadImpl(void *output, size_t byteSize) override
    {
        if(RemainingByteSize() < byteSize)
            return false;

        char *outputData = reinterpret_cast<char*>(output);
        while(byteSize-- > 0)
            *outputData++ = *pData_++;

        return true;
    }

public:

    BinaryMemoryDeserializer(const void *pData, size_t byteSize)
        : pData_(reinterpret_cast<const char*>(pData)),
          pEnd_(reinterpret_cast<const char*>(pData) + byteSize)
    {

    }

    size_t RemainingByteSize() const { return pEnd_ - pData_; }

    bool End() override { return pData_ >= pEnd_; }
};

/**
 * @brief 二进制标准流反序列化器，即将从std::istream中反序列化得到对象的BinaryDeserializer
 */
class BinaryIStreamDeserializer : public BinaryDeserializer, public Uncopiable
{
    std::istream &is_;

    bool ReadImpl(void *output, size_t byteSize) override
    {
        AGZ_ASSERT(output);
        return static_cast<bool>(is_.read((char*)output, byteSize));
    }

public:

    BinaryIStreamDeserializer(std::istream &is) : is_(is) { }
};

}
