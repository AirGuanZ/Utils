#pragma once

#if !(defined(__gl_h_) || defined(__GL_H__))
#   error "Include gl.h/glew.h before AGZUtils/Utils/GL.h"
#endif

#include <AGZUtils/Utils/Math.h>

namespace AGZ::GL
{

using Math::Vec2;
using Math::Vec3;
using Math::Vec4;

using Mat4f = Math::CM_Mat4<GLfloat>;

using Byte = std::uint8_t;
using Vec2b = Vec2<Byte>;
using Vec3b = Vec3<Byte>;
using Vec4b = Vec4<Byte>;

using Vec2f = Vec2<GLfloat>;
using Vec3f = Vec3<GLfloat>;
using Vec4f = Vec4<GLfloat>;

using Rad = Math::Rad<GLfloat>;
using Deg = Math::Deg<GLfloat>;

using Vec2i = Vec2<GLint>;
using Vec3i = Vec3<GLint>;
using Vec4i = Vec4<GLint>;

/**
 * @brief 对GLuint类型的GL Object Name的直接封装
 */
class GLObject : public Uncopiable
{
protected:

    GLuint handle_ = 0;

public:

    explicit GLObject(GLuint handle = 0) noexcept
        : handle_(handle)
    {

    }

    ~GLObject()
    {
        AGZ_ASSERT(!handle_);
    }

    GLuint GetHandle() const noexcept
    {
        return handle_;
    }
};

/**
 * @brief GLSL Sampler2D变量在CPU一侧的type wrapper
 */
struct Texture2DUnit { GLuint unit; };

/**
 * @cond
 */

namespace Impl
{
    inline void SetUniform(GLint loc, GLfloat x)                              { glUniform1f(loc, x);                               }
    inline void SetUniform(GLint loc, const Vec2f &v)                         { glUniform2f(loc, v.x, v.y);                        }
    inline void SetUniform(GLint loc, const Vec3f &v)                         { glUniform3f(loc, v.x, v.y, v.z);                   }
    inline void SetUniform(GLint loc, const Vec4f &v)                         { glUniform4f(loc, v.x, v.y, v.z, v.w);              }
    inline void SetUniform(GLint loc, GLint x)                                { glUniform1i(loc, x);                               }
    inline void SetUniform(GLint loc, const Vec2i &v)                         { glUniform2i(loc, v.x, v.y);                        }
    inline void SetUniform(GLint loc, const Vec3i &v)                         { glUniform3i(loc, v.x, v.y, v.z);                   }
    inline void SetUniform(GLint loc, const Vec4i &v)                         { glUniform4i(loc, v.x, v.y, v.z, v.w);              }
    inline void SetUniform(GLint loc, const Mat4f &m, bool transpose = false) { glUniformMatrix4fv(loc, 1, transpose, &m.m[0][0]); }
    inline void SetUniform(GLint loc, Texture2DUnit v)                        { SetUniform(loc, static_cast<GLint>(v.unit));       }

    template<typename VarType> struct Var2GL;

    template<> struct Var2GL<GLfloat>       { static constexpr GLenum Type = GL_FLOAT;      static constexpr GLenum UType = GL_FLOAT; static constexpr GLint USize = 1; };
    template<> struct Var2GL<GLint>         { static constexpr GLenum Type = GL_INT;        static constexpr GLenum UType = GL_INT;   static constexpr GLint USize = 1; };
    template<> struct Var2GL<Vec2f>         { static constexpr GLenum Type = GL_FLOAT_VEC2; static constexpr GLenum UType = GL_FLOAT; static constexpr GLint USize = 2; };
    template<> struct Var2GL<Vec3f>         { static constexpr GLenum Type = GL_FLOAT_VEC3; static constexpr GLenum UType = GL_FLOAT; static constexpr GLint USize = 3; };
    template<> struct Var2GL<Vec4f>         { static constexpr GLenum Type = GL_FLOAT_VEC4; static constexpr GLenum UType = GL_FLOAT; static constexpr GLint USize = 4; };
    template<> struct Var2GL<Vec2i>         { static constexpr GLenum Type = GL_INT_VEC2;   static constexpr GLenum UType = GL_INT;   static constexpr GLint USize = 2; };
    template<> struct Var2GL<Vec3i>         { static constexpr GLenum Type = GL_INT_VEC3;   static constexpr GLenum UType = GL_INT;   static constexpr GLint USize = 3; };
    template<> struct Var2GL<Vec4i>         { static constexpr GLenum Type = GL_INT_VEC4;   static constexpr GLenum UType = GL_INT;   static constexpr GLint USize = 4; };
    template<> struct Var2GL<Mat4f>         { static constexpr GLenum Type = GL_FLOAT_MAT4; };
    template<> struct Var2GL<Texture2DUnit> { static constexpr GLenum Type = GL_SAMPLER_2D; };
}

/**
 * @endcond
 */

} // namespace AGZ::GL
