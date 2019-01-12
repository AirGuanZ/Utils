#pragma once

#include "Scalar.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

namespace AGZ::Math
{
    
/**
 * @brief ÁÐÖ÷Ðò4x4¾ØÕó
 */
template<typename T>
class CM_Mat4
{
public:

    using Col  = Vec4<T>;
    using Data = Col[4];
    using Self = CM_Mat4<T>;

    Data m;

    constexpr CM_Mat4() noexcept;

    explicit constexpr CM_Mat4(T v) noexcept;

    explicit CM_Mat4(Uninitialized_t) noexcept { }

    explicit CM_Mat4(const Data &_m) noexcept;

    constexpr CM_Mat4(T r0c0, T r0c1, T r0c2, T r0c3,
                      T r1c0, T r1c1, T r1c2, T r1c3,
                      T r2c0, T r2c1, T r2c2, T r2c3,
                      T r3c0, T r3c1, T r3c2, T r3c3) noexcept;

    static constexpr Self FromRows(const Vec4<T> &r0, const Vec4<T> &r1, const Vec4<T> &r2, const Vec4<T> &r3) noexcept;
    
    static constexpr Self FromCols(const Vec4<T> &c0, const Vec4<T> &c1, const Vec4<T> &c2, const Vec4<T> &c3) noexcept;

    static constexpr Self All(T v) noexcept;

    static constexpr Self IDENTITY() noexcept;

    Self operator*(const Self &rhs) const noexcept;

    Vec4<T> operator*(const Vec4<T> &p) const noexcept;

          Col &operator[](size_t idx)       noexcept { AGZ_ASSERT(idx < 4); return m[idx]; }
    const Col &operator[](size_t idx) const noexcept { AGZ_ASSERT(idx < 4); return m[idx]; }

          T &operator()(size_t row, size_t col)       noexcept { AGZ_ASSERT(row < 4 && col < 4); return m[col][row]; }
    const T &operator()(size_t row, size_t col) const noexcept { AGZ_ASSERT(row < 4 && col < 4); return m[col][row]; }

    static Self Translate(const Vec3<T> &v) noexcept;

    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self Rotate(const Vec3<T> &_axis, U angle) noexcept;

    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self RotateX(U angle) noexcept;

    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self RotateY(U angle) noexcept;

    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self RotateZ(U angle) noexcept;

    static Self Scale(const Vec3<T> &s) noexcept;

    template<typename U, std::enable_if_t<IsAngleType_v<U>, int> = 0>
    static Self Perspective(U fovY, T ratio, T _near, T _far) noexcept;

    static Self LookAt(const Vec3<T> &src, const Vec3<T> &dst, const Vec3<T> &up) noexcept;

    Vec3<T> ApplyInverseToNormal(const Vec3<T> &n) const noexcept;

    Self Transpose() const noexcept;

    std::enable_if_t<std::is_floating_point_v<T>, Self> Inverse() const noexcept;
};

using CM_Mat4f = CM_Mat4<float>;
using CM_Mat4d = CM_Mat4<double>;

template<typename T>
CM_Mat4<T> operator*(T lhs, const CM_Mat4<T> &rhs) noexcept
{
    return CM_Mat4<T>::FromCols(lhs * rhs[0], lhs * rhs[1], lhs * rhs[2], lhs * rhs[3]);
}

template<typename T>
CM_Mat4<T> operator*(const CM_Mat4<T> &lhs, T rhs) noexcept
{
    return rhs * lhs;
}

template<typename T>
constexpr CM_Mat4<T>::CM_Mat4() noexcept
    : CM_Mat4(T(1))
{

}

template<typename T>
constexpr CM_Mat4<T>::CM_Mat4(T v) noexcept
    : m{ { v, T(0), T(0), T(0) },
         { T(0), v, T(0), T(0) },
         { T(0), T(0), v, T(0) },
         { T(0), T(0), T(0), v } }
{

}

template<typename T>
CM_Mat4<T>::CM_Mat4(const Data &_m) noexcept
    : m(_m)
{

}

template<typename T>
constexpr CM_Mat4<T>::CM_Mat4(T r0c0, T r0c1, T r0c2, T r0c3,
                              T r1c0, T r1c1, T r1c2, T r1c3,
                              T r2c0, T r2c1, T r2c2, T r2c3,
                              T r3c0, T r3c1, T r3c2, T r3c3) noexcept
    : m{ { r0c0, r1c0, r2c0, r3c0 },
         { r0c1, r1c1, r2c1, r3c1 },
         { r0c2, r1c2, r2c2, r3c2 },
         { r0c3, r1c3, r2c3, r3c3 } }
{

}

template<typename T>
constexpr CM_Mat4<T> CM_Mat4<T>::FromRows(const Vec4<T> &r0, const Vec4<T> &r1, const Vec4<T> &r2, const Vec4<T> &r3) noexcept
{
    return Self(r0[0], r0[1], r0[2], r0[3],
                r1[0], r1[1], r1[2], r1[3],
                r2[0], r2[1], r2[2], r2[3],
                r3[0], r3[1], r3[2], r3[3]);
}

template<typename T>
constexpr CM_Mat4<T> CM_Mat4<T>::FromCols(const Vec4<T> &c0, const Vec4<T> &c1, const Vec4<T> &c2, const Vec4<T> &c3) noexcept
{
    return Self(c0[0], c1[0], c2[0], c3[0],
                c0[1], c1[1], c2[1], c3[1],
                c0[2], c1[2], c2[2], c3[2],
                c0[3], c1[3], c2[3], c3[3]);
}

template<typename T>
constexpr CM_Mat4<T> CM_Mat4<T>::All(T v) noexcept
{
    return Self(v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v);
}

template<typename T>
constexpr CM_Mat4<T> CM_Mat4<T>::IDENTITY() noexcept
{
    return Self(T(1));
}

template<typename T>
CM_Mat4<T> CM_Mat4<T>::operator*(const Self &rhs) const noexcept
{
    Self ret(UNINITIALIZED);
    for(int c = 0; c < 4; ++c)
    {
        for(int r = 0; r < 4; ++r)
        {
            ret.m[c][r] = m[0][r] * rhs.m[c][0]
                        + m[1][r] * rhs[c][1]
                        + m[2][r] * rhs[c][2]
                        + m[3][r] * rhs[c][3];
        }
    }
    return ret;
}

template<typename T>
Vec4<T> CM_Mat4<T>::operator*(const Vec4<T> &p) const noexcept
{
    return Vec4<T>(m[0][0] * p.x + m[1][0] * p.y + m[2][0] * p.z + m[3][0] * p.w,
                   m[0][1] * p.x + m[1][1] * p.y + m[2][1] * p.z + m[3][1] * p.w,
                   m[0][2] * p.x + m[1][2] * p.y + m[2][2] * p.z + m[3][2] * p.w,
                   m[0][3] * p.x + m[1][3] * p.y + m[2][3] * p.z + m[3][3] * p.w);
}

template<typename T>
CM_Mat4<T> CM_Mat4<T>::Translate(const Vec3<T> &v) noexcept
{
    constexpr T I = T(1), O = T(0);
    return Self(I, O, O, v.x,
                O, I, O, v.y,
                O, O, I, v.z,
                O, O, O, I);
}

template<typename T>
template<typename U, std::enable_if_t<IsAngleType_v<U>, int>>
CM_Mat4<T> CM_Mat4<T>::Rotate(const Vec3<T> &_axis, U angle) noexcept
{
    Vec3<T> axis = _axis.Normalize();
    T sinv = Sin(angle), cosv = Cos(angle);

    constexpr T I = T(1), O = T(0);

    T rc[4][4];

    rc[0][0] = axis.x * axis.x + (I - axis.x * axis.x) * cosv;
    rc[0][1] = axis.x * axis.y * (I - cosv) - axis.z * sinv;
    rc[0][2] = axis.x * axis.z * (I - cosv) + axis.y * sinv;
    rc[0][3] = O;

    rc[1][0] = axis.x * axis.y * (I - cosv) + axis.z * sinv;
    rc[1][1] = axis.y * axis.y + (I - axis.y * axis.y) * cosv;
    rc[1][2] = axis.y * axis.z * (I - cosv) - axis.x * sinv;
    rc[1][3] = O;

    rc[2][0] = axis.x * axis.z * (I - cosv) - axis.y * sinv;
    rc[2][1] = axis.y * axis.z * (I - cosv) + axis.x * sinv;
    rc[2][2] = axis.z * axis.z + (I - axis.z * axis.z) * cosv;
    rc[2][3] = O;

    rc[3][0] = O;
    rc[3][1] = O;
    rc[3][2] = O;
    rc[3][3] = I;

    return Self(rc[0][0], rc[0][1], rc[0][2], rc[0][3],
                rc[1][0], rc[1][1], rc[1][2], rc[1][3],
                rc[2][0], rc[2][1], rc[2][2], rc[2][3],
                rc[3][0], rc[3][1], rc[3][2], rc[3][3]);
}

template<typename T>
template<typename U, std::enable_if_t<IsAngleType_v<U>, int>>
CM_Mat4<T> CM_Mat4<T>::RotateX(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(I, O, O, O,
                O, C, -S, O,
                O, S, C, O,
                O, O, O, I);
}

template <typename T>
template <typename U, std::enable_if_t<IsAngleType_v<U>, int>>
CM_Mat4<T> CM_Mat4<T>::RotateY(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(C, O, S, O,
                O, I, O, O,
                -S, O, C, O,
                O, O, O, I);
}

template <typename T>
template <typename U, std::enable_if_t<IsAngleType_v<U>, int>>
CM_Mat4<T> CM_Mat4<T>::RotateZ(U angle) noexcept
{
    constexpr T I = T(1), O = T(0);
    const auto S = Sin(angle), C = Cos(angle);
    return Self(C, -S, O, O,
                S, C, O, O,
                O, O, I, O,
                O, O, O, I);
}

template<typename T>
CM_Mat4<T> CM_Mat4<T>::Scale(const Vec3<T> &s) noexcept
{
    constexpr T I = T(1), O = T(0);
    return Self(s.x, O, O, O,
                O, s.y, O, O,
                O, O, s.z, O,
                O, O, O, I);
}

template<typename T>
template<typename U, std::enable_if_t<IsAngleType_v<U>, int>>
CM_Mat4<T> CM_Mat4<T>::Perspective(U fovY, T ratio, T _near, T _far) noexcept
{
    T invDis = T(1) / (_far - _near);
    constexpr T I = T(1), O = T(0);
    auto cot = Cot(T(0.5) * fovY);
    return Self(cot / ratio, O,   O,             O,
                O,           cot, O,             O,
                O,           O,   _far * invDis, -_far * _near * invDis,
                O,           O,   I,             O);
}

template<typename T>
CM_Mat4<T> CM_Mat4<T>::LookAt(const Vec3<T> &src, const Vec3<T> &dst, const Vec3<T> &up) noexcept
{
    constexpr T I = T(1), O = T(0);
    auto D = (dst - src).Normalize();
    auto R = Cross(up, D).Normalize();
    auto U = Cross(D, R);
    return Self(R.x, U.x, D.x, src.x,
                R.y, U.y, D.y, src.y,
                R.z, U.z, D.z, src.z,
                O,   O,   O,   I).Inverse();
}

template<typename T>
Vec3<T> CM_Mat4<T>::ApplyInverseToNormal(const Vec3<T> &n) const noexcept
{
    return Vec3<T>(m[0][0] * n.x + m[0][1] * n.y + m[0][2] * n.z,
                   m[1][0] * n.x + m[1][1] * n.y + m[1][2] * n.z,
                   m[2][0] * n.x + m[2][1] * n.y + m[2][2] * n.z);
}

template<typename T>
CM_Mat4<T> CM_Mat4<T>::Transpose() const noexcept
{
    return Self(m[0][0], m[0][1], m[0][2], m[0][3],
                m[1][0], m[1][1], m[1][2], m[1][3],
                m[2][0], m[2][1], m[2][2], m[2][3],
                m[3][0], m[3][1], m[3][2], m[3][3]);
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, CM_Mat4<T>> CM_Mat4<T>::Inverse() const noexcept
{
    T coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    T coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    T coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

    T coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    T coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    T coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

    T coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    T coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    T coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

    T coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    T coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
    T coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

    T coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    T coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
    T coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

    T coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    T coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
    T coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

    Vec4<T> fac0(coef00, coef00, coef02, coef03);
    Vec4<T> fac1(coef04, coef04, coef06, coef07);
    Vec4<T> fac2(coef08, coef08, coef10, coef11);
    Vec4<T> fac3(coef12, coef12, coef14, coef15);
    Vec4<T> fac4(coef16, coef16, coef18, coef19);
    Vec4<T> fac5(coef20, coef20, coef22, coef23);

    Vec4<T> vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
    Vec4<T> vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
    Vec4<T> vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
    Vec4<T> vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

    Vec4<T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
    Vec4<T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
    Vec4<T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
    Vec4<T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

    Vec4<T> signA(+1, -1, +1, -1);
    Vec4<T> signB(-1, +1, -1, +1);
    Self inverse = Self::FromCols(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

    Vec4<T> row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

    Vec4<T> dot0(m[0] * row0);
    T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    T invDet = T(1) / dot1;

    return inverse * invDet;
}

} // namespace AGZ::Math
