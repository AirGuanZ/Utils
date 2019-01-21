#pragma once

#include "Common.h"

namespace AGZ::GraphicsAPI::GL
{

/**
 * @brief Attribute Variable Location Wrapper
 * @tparam VarType GLSL变量在CPU一侧的对应类型
 */
template<typename VarType>
class AttribVariable
{
    friend class Program;

    GLuint loc_;

    explicit AttribVariable(GLuint loc) noexcept
        : loc_(loc)
    {

    }

public:

    AttribVariable() noexcept
        : AttribVariable(0)
    {
        
    }

    /**
     * @brief 取得loc值
     */
    GLint GetLocation() const noexcept
    {
        return loc_;
    }
};

} // namespace AGZ::GraphicsAPI::GL
