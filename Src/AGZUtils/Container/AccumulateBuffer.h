#pragma once

#include <iterator>

#include "../Alloc/ObjArena.h"

namespace AGZ
{
    
/**
 * @brief 
 */
template<typename TValue, size_t TSectionSize>
class AccumulateBuffer
{
    struct SectionHead
    {
        SectionHead *next;
    };

    static_assert(TSectionSize > 0);

    SectionHead *entry_; // section入口
    SectionHead *cur_;   // 当前活跃的section
    TValue *nextVal_;    // 下一个val要放哪。只在curRest_ > 0时有意义
    size_t curRest_;     // 当前活跃的section还剩多少空间

    // 申请一块新的buffer section
    void AllocNewSection()
    {
        AGZ_ASSERT(!curRest_);
        size_t valueSize = sizeof(TValue) * TSectionSize;
        size_t totalSize = valueSize + sizeof(SectionHead);

        char *data  = static_cast<char*>(operator new(totalSize));
        auto *value = static_cast<TValue*>(data);
        auto *head  = static_cast<SectionHead*>(data + valueSize);

        if(cur_)
            cur_->next = head;
        else
            entry_ = head;
        head->next = nullptr;

        cur_     = head;
        nextVal_ = value;
        curRest_ = TSectionSize;
    }

    // 释放一个section。只有前valCount个元素会被调用析构函数
    void FreeSection(SectionHead *head, size_t valCount)
    {
        TValue *values = static_cast<TValue*>(
            static_cast<char*>(head) - sizeof(TSectionSize) * sizeof(TValue));
        for(size_t i = 0; i < valCount; ++i)
            (values + i)->~TValue();
        operator delete(values);
    }

public:

    using Value = TValue;
    using Self  = AccumulateBuffer<TValue, TSectionSize>;
    static constexpr size_t SectionSize = TSectionSize;

    AccumulateBuffer() noexcept
        : entry_(nullptr), cur_(nullptr), nextVal_(nullptr), curRest_(0)
    {
        
    }

    AccumulateBuffer(const Self &copyFrom)
        : AccumulateBuffer()
    {
        for(auto &value : copyFrom)
            Push(value);
    }

    AccumulateBuffer(Self &&moveFrom) noexcept
        : entry_(moveFrom.entry_), cur_(moveFrom.cur_),
          nextVal_(moveFrom.nextVal_), curRest_(moveFrom.curRest_)
    {
        moveFrom.entry_   = nullptr;
        moveFrom.cur_     = nullptr;
        moveFrom.nextVal_ = nullptr;
        moveFrom.curRest_ = 0;
    }

    Self &operator=(const Self &copyFrom)
    {
        Clear();
        for(auto &value : copyFrom)
            Push(value);
        return *this;
    }

    Self &operator=(Self &&moveFrom) noexcept
    {
        Clear();

        entry_   = moveFrom.entry_;
        cur_     = moveFrom.cur_;
        nextVal_ = moveFrom.nextVal_;
        curRest_ = moveFrom.curRest_;

        moveFrom.entry_ = nullptr;
        moveFrom.cur_ = nullptr;
        moveFrom.nextVal_ = nullptr;
        moveFrom.curRest_ = 0;

        return *this;
    }

    ~AccumulateBuffer()
    {
        Clear();
    }

    void Push(const TValue &value)
    {
        if(!curRest_)
            AllocNewSection();

        new(nextVal_) TValue(value);
        
        ++nextVal_;
        --curRest_;
    }

    void Push(TValue &&value)
    {
        if(!curRest_)
            AllocNewSection();

        new(nextVal_) TValue(std::move(value));

        ++nextVal_;
        --curRest_;
    }

    template<typename...Args>
    void Emplace(Args&&...args)
    {
        if(!curRest_)
            AllocNewSection();

        new(nextVal_) TValue(std::forward<Args>(args)...);

        ++nextVal_;
        --curRest_;
    }

    void Clear() noexcept
    {
        if(!entry_)
            return;

        SectionHead *head = entry_, *nextHead;
        for(; head->next; head = nextHead)
        {
            nextHead = head->next;
            FreeSection(head, TSectionSize);
        }
        FreeSection(head, TSectionSize - curRest_);

        entry_   = nullptr;
        cur_     = nullptr;
        curRest_ = 0;
        nextVal_ = nullptr;
    }

    class Iterator
    {
        TValue *values_;
        size_t curPos_;

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = TValue;
        using difference_type = std::ptrdiff_t;
        using pointer = TValue * ;
        using reference = TValue & ;

        Iterator(TValue *values, size_t curPos) noexcept
            : values_(values), curPos_(curPos)
        {

        }

        value_type &operator*() { return values_[curPos_]; }
        pointer operator->() { return values_ + curPos_; }

        Iterator &operator++()
        {
            if(++curPos_ >= TSectionSize)
            {
                auto head = static_cast<SectionHead*>(
                    static_cast<char*>(values_) + sizeof(TValue) * TSectionSize);
                head = head->next;
                values_ = static_cast<TValue*>(
                    static_cast<char*>(head) - sizeof(TValue) * TSectionSize);
                curPos_ = 0;
            }
            return *this;
        }

        Iterator operator++(int)
        {
            auto ret = *this;
            ++*this;
            return ret;
        }

        bool operator==(const Iterator &rhs) const noexcept
        {
            return values_ == rhs.values_ && curPos_ == rhs.curPos_;
        }

        bool operator!=(const Iterator &rhs) const noexcept
        {
            return !(*this == rhs);
        }
    };

    Iterator begin()
    {
        return Iterator(entry_, 0);
    }

    Iterator end()
    {
        return Iterator(cur_, TSectionSize - curRest_);
    }
};

} // namespace AGZ
