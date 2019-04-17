#pragma once

#include "../Utils/Math.h"

namespace AGZ::GraphicsAPI
{

namespace GL
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

using Vec2i = Vec2<int>;
using Vec3i = Vec3<int>;
using Vec4i = Vec4<int>;

using Rad = Math::Rad<float>;
using Deg = Math::Deg<float>;

} // namespace GL

} // namespace AGZ::GraphicsAPI
