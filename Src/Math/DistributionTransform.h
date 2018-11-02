#pragma once

#include "../Misc/Common.h"
#include "Tri.h"
#include "Vec2.h"
#include "Vec3.h"

AGZ_NS_BEG(AGZ::Math::DistributionTransform)

template<typename T> class UniformOnUnitDisk;
template<typename T> class UniformOnUnitHemisphere;
template<typename T> class UniformOnUnitSphere;
template<typename T> class ZWeightedOnUnitHemisphere;
template<typename T> class ZWeightedOnUnitSphere;
template<typename T> class UniformOnTriangle;
template<typename T> class UniformOnCone;

template<typename T>
class UniformOnUnitDisk
{
public:

    struct Result
    {
        Vec2<T> sample;
        T pdf;
    };

    static Result Transform(const Vec2<T> &u)
    {
        T r = Sqrt(u.u);
        T theta = 2 * PI<T> * u.v;
        return { { r * Cos(theta), r*  Sin(theta) }, PI<T> };
    }

    static T PDF()
    {
        return PI<T>;
    }
};

template<typename T>
class UniformOnUnitHemisphere
{
public:

    struct Result
    {
        Vec3<T> sample;
        T pdf;
    };

    static Result Transform(const Vec2<T> &u)
    {
        T z = u.u;
        T phi = 2 * PI<T> * u.v;
        T r = Sqrt(Max(T(0), 1 - z * z));
        T x = r * Cos(phi);
        T y = r * Sin(phi);
        return { { x, y, z }, Inv2PI<T> };
    }

    static T PDF()
    {
        return Inv2PI<T>;
    }
};

template<typename T>
class UniformOnUnitSphere
{
public:

    struct Result
    {
        Vec3<T> sample;
        T pdf;
    };

    static Result Transform(const Vec2<T> &u)
    {
        T z = 1 - 2 * u.u;
        T phi = 2 * PI<T> * u.v;
        T r = Sqrt(Max(T(0), 1 - z * z));
        T x = r * Cos(phi);
        T y = r * Sin(phi);
        return { { x, y, z }, Inv4PI<T> };
    }

    static T PDF()
    {
        return Inv4PI<T>;
    }
};

template<typename T>
class ZWeightedOnUnitHemisphere
{
public:

    struct Result
    {
        Vec3<T> sample;
        T pdf;
    };

    static Result Transform(Vec2<T> u)
    {
        Vec2<T> sam;
        u = T(2) * u - Vec2<T>(1);
        if(u.x == 0 && u.y == 0)
            sam = Vec2<T>(T(0));
        else
        {
            T theta, r;
            if(Abs(u.x) > Abs(u.y)) {
                r = u.x;
                theta = T(0.25) * PI<T> * (u.y / u.x);
            }
            else {
                r = u.y;
                theta = T(0.5) * PI<T> - T(0.25) * PI<T> * (u.x / u.y);
            }
            sam = r * Vec2<T>(Cos(theta), Sin(theta));
        }

        T z = Sqrt(Max(T(0), 1 - sam.LengthSquare()));
        return { { sam.x, sam.y, z }, z * InvPI<T> };
    }

    static T PDF(const Vec3<T> &sample)
    {
        return sample.z * InvPI<T>;
    }
};

template<typename T>
class UniformOnTriangle
{
public:

    static Vec2<T> Transform(const Vec2<T> &u)
    {
        T t = Sqrt(u.u);
        return { 1 - t, t * u.v };
    }
};

template<typename T>
class UniformOnCone
{
public:

    struct Result
    {
        Vec3<T> sample;
        T pdf;
    };

    Result Transform(T maxCosTheta, const Vec2<T> &u)
    {
        T cosTheta = (1 - u.u) + u.u * maxCosTheta;
        T sinTheta = Sqrt(Max(T(0), T(1) - cosTheta * cosTheta));
        T phi = 2 * PI<T> * u.v;
        return {
            { Cos(phi) * sinTheta, Sin(phi) * sinTheta, cosTheta },
            1 / (2 * PI<T> * (1 - maxCosTheta))
        };
    }

    static T PDF(T maxCosTheta)
    {
        return 1 / (2 * PI<T> * (1 - maxCosTheta));
    }
};

AGZ_NS_END(AGZ::Math::DistributionTransform)
