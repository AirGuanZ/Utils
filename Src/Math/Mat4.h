#pragma once

#include <cstring>
#include <type_traits>

#include "../Common.h"
#include "Scalar.h"
#include "Vec4.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

/*
    Row-major 4x4 matrix
*/
template<typename T>
class Mat4
{
public:

    T m[4][4];

public:

    using Component = T;
    using Self = Mat4<T>;

    static_assert(std::is_trivially_copyable<Component>::value,
                  "Matrix component must be trivially copyable");

    Mat4(Uninitialized_t)
    {

    }

    Mat4()
        : Mat4(Scalar::ONE<T>())
    {

    }

    Mat4(const T &v)
    {
        m[0][1] = m[0][2] = m[0][3] =
        m[1][0] = m[1][2] = m[1][3] =
        m[2][0] = m[2][1] = m[2][3] =
        m[3][0] = m[3][1] = m[3][2] = Scalar::ZERO<T>();
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = v;
    }

    Mat4(const (&_m)[4][4])
    {
        std::memcpy(m, _m, sizeof(m));
    }

    Mat4(T m00, T m01, T m02, T m03,
         T m10, T m11, T m12, T m13,
         T m20, T m21, T m22, T m23,
         T m30, T m31, T m32, T m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    static Self All(T v)
    {
        Self ret(UNINITIALIZED);
        m[0][0] = m[0][1] = m[0][2] = m[0][3] =
        m[1][0] = m[1][1] = m[1][2] = m[1][3] =
        m[2][0] = m[2][1] = m[2][2] = m[2][3] =
        m[3][0] = m[3][1] = m[3][2] = m[3][3] = v;
        return ret;
    }

    static const Self &IDENTITY()
    {
        static const Self ret;
        return ret;
    }

    bool operator==(const Self &other) const
    {
        for(int c = 0; c < 4; ++c)
        {
            for(int r = 0; r < 4; ++r)
            {
                if(m[c][r] != other[c][r])
                    return false;
            }
        }
        return true;
    }

    bool operator!=(const Self &other) const
    {
        for(int c = 0; c < 4; ++c)
        {
            for(int r = 0; r < 4; ++r)
            {
                if(m[c][r] != other[c][r])
                    return true;
            }
        }
        return false;
    }

    Self operator*(const Self &rhs) const
    {
        Self ret(UNINITIALIZED);
        for(int r = 0; r < 4; ++r)
        {
            for(int c = 0; c < 4; ++c)
            {
                ret.m[r][c] = m[r][0] * rhs.m[0][c]
                            + m[r][1] * rhs.m[1][c]
                            + m[r][2] * rhs.m[2][c]
                            + m[r][3] * rhs.m[3][c];
            }
        }
        return ret;
    }
    
    Vec4<T> operator*(const Vec4<T> &p)
    {
        return Vec4<T>(m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3] * p.w,
                       m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3] * p.w,
                       m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3] * p.w,
                       m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3] * p.w
    }

    Vec4<T> ApplyToPoint(const Vec4<T> &p)
    {
        return *this * p;
    }

    Vec3<T> ApplyToPoint(const Vec3<T> &p)
    {
        Vec4<T> ret = *this * Vec4<T>(p.x, p.y, p.z, 1.0);
        T dw = Scalar::ONE<T>() / ret.w;
        return Vec3<T>(dw * ret.x, dw * ret.y, dw * ret.z);
    }

    Vec4<T> ApplyToVector(const Vec4<T> &v)
    {
        return *this * v;
    }

    Vec4<T> ApplyToVector(const Vec3<T> &v)
    {
        return Vec3<T>(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                       m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                       m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
    }
};

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
