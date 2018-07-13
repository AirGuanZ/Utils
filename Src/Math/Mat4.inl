#include <algorithm>

#include "Scalar.h"

AGZ_NS_BEG(AGZ)
AGZ_NS_BEG(Math)

template<typename T>
inline Mat4<T>::Mat4(T v)
{
    m[0][1] = m[0][2] = m[0][3] =
    m[1][0] = m[1][2] = m[1][3] =
    m[2][0] = m[2][1] = m[2][3] =
    m[3][0] = m[3][1] = m[3][2] = ZERO<T>();
    m[0][0] = m[1][1] = m[2][2] = m[3][3] = v;
}

template<typename T>
inline Mat4<T>::Mat4(const typename Mat4<T>::Data &_m)
{
    static_assert(std::is_trivially_copyable<Component>::value);
    std::memcpy(m, _m, sizeof(m));
}

template<typename T>
inline Mat4<T>::Mat4(T m00, T m01, T m02, T m03,
                     T m10, T m11, T m12, T m13,
                     T m20, T m21, T m22, T m23,
                     T m30, T m31, T m32, T m33)
{
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

template<typename T>
inline Mat4<T> Mat4<T>::All(T v)
{
    Self ret(UNINITIALIZED);
    m[0][0] = m[0][1] = m[0][2] = m[0][3] =
    m[1][0] = m[1][1] = m[1][2] = m[1][3] =
    m[2][0] = m[2][1] = m[2][2] = m[2][3] =
    m[3][0] = m[3][1] = m[3][2] = m[3][3] = v;
    return ret;
}

template<typename T>
inline const Mat4<T> &Mat4<T>::IDENTITY()
{
    static const Self ret;
    return ret;
}

template<typename T>
inline bool Mat4<T>::operator==(const typename Mat4<T>::Self &other) const
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

template<typename T>
inline bool Mat4<T>::operator!=(const typename Mat4<T>::Self &other) const
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

template<typename T>
inline typename Mat4<T>::Self Mat4<T>::operator*(const typename Mat4<T>::Self &rhs) const
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

template<typename T>
inline Vec4<T> Mat4<T>::operator*(const Vec4<T> &p)
{
    return Vec4<T>(m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3] * p.w,
                   m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3] * p.w,
                   m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3] * p.w,
                   m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3] * p.w);
}

template<typename T>
inline typename Mat4<T>::Self Mat4<T>::Translate(const Vec3<T> &v)
{
    constexpr T I = ONE<T>(), O = ZERO<T>();
    return Self(I, O, O, v.x,
                O, I, O, v.y,
                O, O, I, v.z,
                O, O, O, I);
}

template<typename T>
template<typename U>
inline typename Mat4<T>::Self Mat4<T>::Rotate(const Vec3<T> &_axis, U angle)
{
    T m[4][4];
    Vec3<T> axis = Normalize(_axis);
    auto sinv = Sin(angle), cosv = Cos(angle);

    constexpr T I = ONE<T>(), O = ZERO<T>();

    m[0][0] = axis.x * axis.x + (I - axis.x * axis.x) * cosv;
    m[0][1] = axis.x * axis.y * (I - cosv) - axis.z * sinv;
    m[0][2] = axis.x * axis.z * (I - cosv) + axis.y * sinv;
    m[0][3] = O;

    m[1][0] = axis.x * axis.y * (I - cosv) + axis.z * sinv;
    m[1][1] = axis.y * axis.y + (I - axis.y * axis.y) * cosv;
    m[1][2] = axis.y * axis.z * (I - cosv) - axis.x * sinv;
    m[1][3] = O;

    m[2][0] = axis.x * axis.z * (I - cosv) - axis.y * sinv;
    m[2][1] = axis.y * axis.z * (I - cosv) + axis.x * sinv;
    m[2][2] = axis.z * axis.z + (I - axis.z * axis.z) * cosv;
    m[2][3] = O;

    m[3][0] = O;
    m[3][1] = O;
    m[3][2] = O;
    m[3][3] = I;

    return Mat4<T>(m);
}

template<typename T>
inline typename Mat4<T>::Self Mat4<T>::Scale(const Vec3<T> &s)
{
    constexpr T I = ONE<T>(), O = ZERO<T>();
    return Self(s.x, O, O, O,
                O, s.y, O, O,
                O, O, s.z, O,
                O, O, O,   I);
}

template<typename T>
inline Vec4<T> ApplyToPoint(const Mat4<T> &m, const Vec4<T> &v)
{
    return m * p;
}

template<typename T>
inline Vec3<T> ApplyToPoint(const Mat4<T> &m, const Vec3<T> &p)
{
    Vec4<T> ret = m * Vec4<T>(p.x, p.y, p.z, 1.0);
    T dw = ONE<T>() / ret.w;
    return Vec3<T>(dw * ret.x, dw * ret.y, dw * ret.z);
}

template<typename T>
inline Vec4<T> ApplyToVector(const Mat4<T> &m, const Vec4<T> &v)
{
    return m * v;
}

template<typename T>
inline Vec4<T> ApplyToVector(const Mat4<T> &m, const Vec3<T> &v)
{
    return Vec3<T>(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
                   m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
                   m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
}

template<typename T>
inline Mat4<T> Transpose(const Mat4<T> &m)
{
    return Mat4<T>(m[0][0], m[1][0], m[2][0], m[3][0],
                   m[0][1], m[1][1], m[2][1], m[3][1],
                   m[0][2], m[1][2], m[2][2], m[3][2],
                   m[0][3], m[1][3], m[2][3], m[3][3]);
}

template<>
inline Mat4<float> Inverse<float>(const Mat4<float> &m)
{
    int indxc[4], indxr[4];
    int ipiv[4] = { 0, 0, 0, 0 };
    float minv[4][4];
    std::memcpy(minv, m.m, 4 * 4 * sizeof(float));
    for(int i = 0; i < 4; i++)
    {
        int irow = -1, icol = -1;
        float big = 0.0f;
        for(int j = 0; j < 4; j++)
        {
            if(ipiv[j] != 1)
            {
                for(int k = 0; k < 4; k++)
                {
                    if(ipiv[k] == 0)
                    {
                        if(Abs(minv[j][k]) >= big)
                        {
                            big = Abs(minv[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                    AGZ_ASSERT(ipiv[k] <= 1);
                }
            }
        }
        ++ipiv[icol];
        if(irow != icol)
        {
            for(int k = 0; k < 4; ++k)
                std::swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        AGZ_ASSERT(minv[icol][icol] != 0.0f);

        float pivinv = 1.f / minv[icol][icol];
        minv[icol][icol] = 1.f;
        for(int j = 0; j < 4; j++)
            minv[icol][j] *= pivinv;

        for(int j = 0; j < 4; j++)
        {
            if(j != icol)
            {
                float save = minv[j][icol];
                minv[j][icol] = 0;
                for(int k = 0; k < 4; k++)
                    minv[j][k] -= minv[icol][k] * save;
            }
        }
    }
    for(int j = 3; j >= 0; j--)
    {
        if(indxr[j] != indxc[j])
        {
            for(int k = 0; k < 4; k++)
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }
    return Mat4<float>(minv);
}

template<>
inline Mat4<double> Inverse<double>(const Mat4<double> &m)
{
    int indxc[4], indxr[4];
    int ipiv[4] = { 0, 0, 0, 0 };
    double minv[4][4];
    std::memcpy(minv, m.m, 4 * 4 * sizeof(double));
    for(int i = 0; i < 4; i++)
    {
        int irow = -1, icol = -1;
        double big = 0.0;
        for(int j = 0; j < 4; j++)
        {
            if(ipiv[j] != 1)
            {
                for(int k = 0; k < 4; k++)
                {
                    if(ipiv[k] == 0)
                    {
                        if(Abs(minv[j][k]) >= big)
                        {
                            big = Abs(minv[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                    AGZ_ASSERT(ipiv[k] <= 1);
                }
            }
        }
        ++ipiv[icol];
        if(irow != icol)
        {
            for(int k = 0; k < 4; ++k)
                std::swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        AGZ_ASSERT(minv[icol][icol] != 0.0);

        double pivinv = 1.0 / minv[icol][icol];
        minv[icol][icol] = 1.0;
        for(int j = 0; j < 4; j++)
            minv[icol][j] *= pivinv;

        for(int j = 0; j < 4; j++)
        {
            if(j != icol)
            {
                double save = minv[j][icol];
                minv[j][icol] = 0;
                for(int k = 0; k < 4; k++)
                    minv[j][k] -= minv[icol][k] * save;
            }
        }
    }
    for(int j = 3; j >= 0; j--)
    {
        if(indxr[j] != indxc[j])
        {
            for(int k = 0; k < 4; k++)
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }
    return Mat4<double>(minv);
}

AGZ_NS_END(Math)
AGZ_NS_END(AGZ)
