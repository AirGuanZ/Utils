#pragma once

#include "Texture.h"

namespace AGZ {

/**
 * @brief 最近邻纹理采样器
 */
class NearestSampler
{
public:

	/**
	 * @brief 给定二维纹理和纹理坐标，对其进行最近邻采样
	 * 
	 * @param tex 纹理对象
	 * @param texCoord 纹理坐标，取值范围为[0, 1]^2，(0, 0)对应纹理左上角
	 */
    template<typename PT, typename E>
    static auto Sample(const Texture2D<PT> &tex, const Math::Vec2<E> &texCoord)
    {
        return tex(
            Math::Clamp(static_cast<int>(texCoord.u * tex.GetWidth()),
                        0, static_cast<int>(tex.GetWidth()  - 1)),
            Math::Clamp(static_cast<int>(texCoord.v * tex.GetHeight()),
                        0, static_cast<int>(tex.GetHeight() - 1)));
    }
};

/**
 * @brief 双线性插值采样器
 */
class LinearSampler
{
    template<typename PT, typename E>
    static AGZ_FORCEINLINE auto Interpolate(
        const PT &u0v0, const PT &u1v0, const PT &u0v1, const PT &u1v1,
        E u, E v)
    {
        return (u0v0 * (1 - u) + u1v0 * u) * (1 - v)
             + (u0v1 * (1 - u) + u1v1 * u) * v;
    }

public:

	/**
	 * @brief 给定二维纹理和纹理坐标，对其进行双线性采样
	 *
	 * @param tex 纹理对象
	 * @param texCoord 纹理坐标，取值范围为[0, 1]^2，(0, 0)对应纹理左上角
	 */
    template<typename PT, typename E>
    static auto Sample(const Texture2D<PT> &tex, const Math::Vec2<E> &texCoord)
    {
        auto fu = static_cast<E>(texCoord.u * tex.GetWidth());
        auto fv = static_cast<E>(texCoord.v * tex.GetHeight());

        int pu = Math::Clamp(static_cast<int>(fu), 0, static_cast<int>(tex.GetWidth() - 1));
        int pv = Math::Clamp(static_cast<int>(fv), 0, static_cast<int>(tex.GetHeight() - 1));

        int dpu = (fu > pu + E(0.5)) ? 1 : -1;
        int dpv = (fv > pv + E(0.5)) ? 1 : -1;

        int apu = Math::Clamp(pu + dpu, 0, static_cast<int>(tex.GetWidth() - 1));
        int apv = Math::Clamp(pv + dpv, 0, static_cast<int>(tex.GetHeight() - 1));

        E du = Math::Min(Math::Abs(fu - pu - E(0.5)), E(1));
        E dv = Math::Min(Math::Abs(fv - pv - E(0.5)), E(1));

        return Interpolate(tex(pu, pv), tex(apu, pv), tex(pu, apv), tex(apu, apv), du, dv);
    }
};

} // namespace AGZ
