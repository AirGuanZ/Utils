#pragma once

#ifdef AGZ_USE_OPENGL

#include "../../../Utils/Math.h"

namespace AGZ::OpenGL
{

struct Sampler2D { GLint value; };

namespace Impl
{
    using Math::Vec2;
    using Math::Vec3;
    using Math::Vec4;

    using Math::Mat4f;
    using Math::Vec2f;
    using Math::Vec3f;
    using Math::Vec4f;

    inline void SetUniform(GLint loc, GLfloat x)                                  { glUniform1f(loc, x);                               }
    inline void SetUniform(GLint loc, GLfloat x, GLfloat y)                       { glUniform2f(loc, x, y);                            }
    inline void SetUniform(GLint loc, GLfloat x, GLfloat y, GLfloat z)            { glUniform3f(loc, x, y, z);                         }
    inline void SetUniform(GLint loc, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { glUniform4f(loc, x, y, z, w);                      }
    inline void SetUniform(GLint loc, const Vec2f &v)                             { SetUniform(loc, v.x, v.y);                         }
    inline void SetUniform(GLint loc, const Vec3f &v)                             { SetUniform(loc, v.x, v.y, v.z);                    }
    inline void SetUniform(GLint loc, const Vec4f &v)                             { SetUniform(loc, v.x, v.y, v.z, v.w);               }
    inline void SetUniform(GLint loc, GLint x)                                    { glUniform1i(loc, x);                               }
    inline void SetUniform(GLint loc, GLint x, GLint y)                           { glUniform2i(loc, x, y);                            }
    inline void SetUniform(GLint loc, GLint x, GLint y, GLint z)                  { glUniform3i(loc, x, y, z);                         }
    inline void SetUniform(GLint loc, GLint x, GLint y, GLint z, GLint w)         { glUniform4i(loc, x, y, z, w);                      }
    inline void SetUniform(GLint loc, const Vec2<GLint> &v)                       { SetUniform(loc, v.x, v.y);                         }
    inline void SetUniform(GLint loc, const Vec3<GLint> &v)                       { SetUniform(loc, v.x, v.y, v.z);                    }
    inline void SetUniform(GLint loc, const Vec4<GLint> &v)                       { SetUniform(loc, v.x, v.y, v.z, v.w);               }
    inline void SetUniform(GLint loc, const Mat4f &m, bool transpose = true)      { glUniformMatrix4fv(loc, 1, transpose, &m.m[0][0]); }

    template<typename...VarTypes> struct Var2GL;

    template<> struct Var2GL<GLfloat>                            { static constexpr GLenum Value = GL_FLOAT;      };
    template<> struct Var2GL<GLfloat, GLfloat>                   { static constexpr GLenum Value = GL_FLOAT_VEC2; };
    template<> struct Var2GL<GLfloat, GLfloat, GLfloat>          { static constexpr GLenum Value = GL_FLOAT_VEC3; };
    template<> struct Var2GL<GLfloat, GLfloat, GLfloat, GLfloat> { static constexpr GLenum Value = GL_FLOAT_VEC4; };
    template<> struct Var2GL<GLint>                              { static constexpr GLenum Value = GL_INT;        };
    template<> struct Var2GL<GLint, GLint>                       { static constexpr GLenum Value = GL_INT_VEC2; };
    template<> struct Var2GL<GLint, GLint, GLint>                { static constexpr GLenum Value = GL_INT_VEC3; };
    template<> struct Var2GL<GLint, GLint, GLint, GLint>         { static constexpr GLenum Value = GL_INT_VEC4; };
    template<> struct Var2GL<Vec2f>                              { static constexpr GLenum Value = GL_FLOAT_VEC2; };
    template<> struct Var2GL<Vec3f>                              { static constexpr GLenum Value = GL_FLOAT_VEC3; };
    template<> struct Var2GL<Vec4f>                              { static constexpr GLenum Value = GL_FLOAT_VEC4; };
    template<> struct Var2GL<Vec2<GLint>>                        { static constexpr GLenum Value = GL_INT_VEC2;   };
    template<> struct Var2GL<Vec3<GLint>>                        { static constexpr GLenum Value = GL_INT_VEC3;   };
    template<> struct Var2GL<Vec4<GLint>>                        { static constexpr GLenum Value = GL_INT_VEC4;   };
    template<> struct Var2GL<Mat4f>                              { static constexpr GLenum Value = GL_FLOAT_MAT3; };
    template<> struct Var2GL<Sampler2D>                          { static constexpr GLenum Value = GL_SAMPLER_2D; };
}

template<typename...VarTypes>
class UniformVariable
{
    GLint loc_;

public:

    explicit UniformVariable(GLint loc) noexcept
        : loc_(loc)
    {
        
    }

    void ApplyValue(const VarTypes &...vars)
    {
        Impl::SetUniform(loc_, vars...);
    }
};

} // namespace AGZ::OpenGL

#endif // #ifdef AGZ_USE_OPENGL
