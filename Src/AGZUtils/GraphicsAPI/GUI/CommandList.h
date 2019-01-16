#pragma once

#include "../../Misc/TypeOpr.h"
#include "../../Utils/Math.h"

namespace AGZ::GUI
{

using Math::Vec2;
using Math::Vec3;
using Math::Vec4;

using Mat4f = Math::CM_Mat4<float>;

using Byte = std::uint8_t;
using Vec2b = Vec2<Byte>;
using Vec3b = Vec3<Byte>;
using Vec4b = Vec4<Byte>;

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;

using Rad = Math::Rad<float>;
using Deg = Math::Deg<float>;

struct CommandType
{
    struct DrawTriangle
    {
        Vec2f A, B, C;
    };

    struct DrawLine
    {
        Vec2f A, B;
    };

    struct DrawPoint
    {
        Vec2f A;
    };

    struct SetColor
    {
        Vec4f color;
    };

    struct EnableDepthTest
    {
        bool enabled;
    };

    struct SetFillMode
    {
        enum Mode { Fill, Line } mode;
    };

    using Command = TypeOpr::Variant<
        DrawTriangle,
        DrawLine,
        DrawPoint,
        SetColor,
        EnableDepthTest,
        SetFillMode
    >;
};

using Command = CommandType::Command;

class CommandBuffer
{
    
};

} // namespace AGZ::GUI
