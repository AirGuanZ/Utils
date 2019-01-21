#pragma once

namespace AGZ {

/**
 * @brief 不可复制类模板
 */
class Uncopiable
{
public:

    Uncopiable()                             = default;
    Uncopiable(const Uncopiable&)            = delete;
    Uncopiable &operator=(const Uncopiable&) = delete;
    Uncopiable(Uncopiable&&) noexcept = default;
    Uncopiable &operator=(Uncopiable&&) noexcept = default;
};

/**
 * @brief 不可移动类模板
 */
class Unmovable
{
public:

    Unmovable() = default;
    Unmovable(Unmovable&&) noexcept = delete;
    Unmovable &operator=(Unmovable&&) noexcept = delete;
};

} // namespace AGZ
