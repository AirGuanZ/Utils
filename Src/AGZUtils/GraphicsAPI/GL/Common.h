#pragma once

#ifndef AGZ_DISABLE_NATIVE_GL_CONTEXT

#if !(defined(__gl_h_) || defined(__GL_H__))
#   error "Include gl.h/glew.h before this header"
#endif

#define AGZ_GL_CTX

#endif // AGZ_DISABLE_NATIVE_GL_CONTEXT

#include "../../Misc/Common.h"
#include "../Common.h"

namespace AGZ::GraphicsAPI::GL
{

/**
 * @brief 对GLuint类型的GL Object Name的直接封装
 * @note 所有的GLObject均独占所有权，不可复制
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
    inline void SetUniform(GLint loc, GLfloat x)                              { AGZ_GL_CTX glUniform1f(loc, x);                               }
    inline void SetUniform(GLint loc, const Vec2f &v)                         { AGZ_GL_CTX glUniform2f(loc, v.x, v.y);                        }
    inline void SetUniform(GLint loc, const Vec3f &v)                         { AGZ_GL_CTX glUniform3f(loc, v.x, v.y, v.z);                   }
    inline void SetUniform(GLint loc, const Vec4f &v)                         { AGZ_GL_CTX glUniform4f(loc, v.x, v.y, v.z, v.w);              }
    inline void SetUniform(GLint loc, GLint x)                                { AGZ_GL_CTX glUniform1i(loc, x);                               }
    inline void SetUniform(GLint loc, const Vec2i &v)                         { AGZ_GL_CTX glUniform2i(loc, v.x, v.y);                        }
    inline void SetUniform(GLint loc, const Vec3i &v)                         { AGZ_GL_CTX glUniform3i(loc, v.x, v.y, v.z);                   }
    inline void SetUniform(GLint loc, const Vec4i &v)                         { AGZ_GL_CTX glUniform4i(loc, v.x, v.y, v.z, v.w);              }
    inline void SetUniform(GLint loc, const Mat4f &m, bool transpose = false) { AGZ_GL_CTX glUniformMatrix4fv(loc, 1, transpose, &m.m[0][0]); }
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

} // namespace AGZ::GraphicsAPI::GL
