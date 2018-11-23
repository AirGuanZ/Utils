#pragma once

#include "../Utils/Math.h"

namespace AGZ {

/**
 * @brief 球面环境光的纹理坐标映射
 * 
 * 把一个理想镜面反射的球体放在场景中间，然后用正交投影摄像机去看它，看到的球面就可以反映环境光。
 * 仔细一想，没错，这居然可以表示完整的environment lighting，很棒的样子。
 * 
 * 不过话说回来，直接把(theta, phi)归一化之后编码成uv有什么不好，搞不懂这个SphereMap有什么意义。
 */
template<typename T>
class SphereMapper
{
public:

    /**
     * 求能给出沿着-dir方向的环境光的texel对应的uv坐标
     * 
     * 假设是沿着+x方向看的球体，球体充斥了整个画面，和图像的四个边缘都正好相切
     */
    static Math::Vec2<T> Map(const Math::Vec3<T> &dir);

    /**
     * 给定SphereMap上的uv坐标，求其对应的dir
     */
    static Math::Vec3<T> InvMap(const Math::Vec2<T> &dir);
};

template<typename T>
Math::Vec2<T> SphereMapper<T>::Map(const Math::Vec3<T> &dir)
{
    // TODO
    return { };
}

} // namespace AGZ
