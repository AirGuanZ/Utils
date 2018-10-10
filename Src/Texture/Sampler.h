#pragma once

#include "Texture.h"

AGZ_NS_BEG(AGZ::Tex)

class NearestSampler
{
public:

    template<typename PT, typename E>
    static auto Sample(const Texture2D<PT> &tex, const Math::Vec2<E> &texCoord)
    {
        return tex(
            Clamp(static_cast<int>(texCoord.u * tex.GetWidth()), 0, tex.GetWidth() - 1),
            Clamp(static_cast<int>(texCoord.v * tex.GetHeight()), 0, tex.GetHeight() - 1));
    }
};

AGZ_NS_END(AGZ::Tex)
