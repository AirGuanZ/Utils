#pragma once

#include "Buffer.h"
#include "Program.h"
#include "ProgramBuilder.h"
#include "RenderContext.h"
#include "Texture2D.h"
#include "VertexArray.h"

namespace AGZ::GraphicsAPI::GL
{
    
/**
 * @brief 用于直接在屏幕上绘制内容的工具
 */
class Immediate2D : public Uncopiable
{
    struct PrimitiveVertex
    {
        Vec2f pos;
    };
    VertexBuffer<PrimitiveVertex> primVtxBuf_;
    ElementBuffer<GLushort> primElemBuf_;

    struct
    {
        Program prog;
        UniformVariable<Vec2f> uniform_A;
        UniformVariable<Vec2f> uniform_B;
        UniformVariable<Vec4f> uniform_FRAG_COLOR;
        VertexArray vao;
    } affine_;

    struct
    {
        Program prog;
        UniformVariable<Vec2f> uniform_A;
        UniformVariable<Vec2f> uniform_B_A;
        UniformVariable<Vec2f> uniform_C_A;
        UniformVariable<Vec4f> uniform_FRAG_COLOR;
        VertexArray vao;
    } triangle_;

    struct
    {
        Program prog_;
        UniformVariable<Texture2DUnit> uniform_TEX;
        AttribVariable<Vec2f> attrib_iPos;
        AttribVariable<Vec2f> attrib_iTexCoord;
        VertexArray vao;
    } texTriangle_;

    Vec2f pixelSize_;

    static constexpr uint32_t VERTEX_QUAD_BEGIN   = 0;
    static constexpr uint32_t VERTEX_QUAD_END     = 4 + VERTEX_QUAD_BEGIN;
    static constexpr uint32_t VERTEX_CIRCLE_BEGIN = VERTEX_QUAD_END;
    static constexpr uint32_t VERTEX_CIRCLE_END   = 256 + VERTEX_CIRCLE_BEGIN;

    static constexpr uint32_t VERTEX_COUNT = VERTEX_CIRCLE_END;

    static constexpr uint32_t ELEM_FILL_QUAD_BEGIN   = 0;
    static constexpr uint32_t ELEM_FILL_QUAD_END     = 6 + ELEM_FILL_QUAD_BEGIN;
    static constexpr uint32_t ELEM_LINE_QUAD_BEGIN   = ELEM_FILL_QUAD_END;
    static constexpr uint32_t ELEM_LINE_QUAD_END     = 4 + ELEM_LINE_QUAD_BEGIN;
    static constexpr uint32_t ELEM_LINE_BEGIN        = ELEM_LINE_QUAD_END;
    static constexpr uint32_t ELEM_LINE_END          = 2 + ELEM_LINE_BEGIN;
    static constexpr uint32_t ELEM_TRIANGLE_BEGIN    = ELEM_LINE_END;
    static constexpr uint32_t ELEM_TRIANGLE_END      = 3 + ELEM_TRIANGLE_BEGIN;
    static constexpr uint32_t ELEM_LINE_CIRCLE_BEGIN = ELEM_TRIANGLE_END;
    static constexpr uint32_t ELEM_LINE_CIRCLE_END   = (VERTEX_CIRCLE_END - VERTEX_CIRCLE_BEGIN) + ELEM_LINE_CIRCLE_BEGIN;
    static constexpr uint32_t ELEM_FILL_CIRCLE_BEGIN = ELEM_LINE_CIRCLE_END;
    static constexpr uint32_t ELEM_FILL_CIRCLE_END   = (VERTEX_CIRCLE_END - VERTEX_CIRCLE_BEGIN) * 3 + ELEM_FILL_CIRCLE_BEGIN;

    static constexpr uint32_t ELEM_COUNT = ELEM_FILL_CIRCLE_END;

    void InitializePrimitiveBuffer()
    {
        /*
            0: (0, 0)
            1: (0, 1)
            2: (1, 1)
            3: (1, 0)
        */
        PrimitiveVertex vtxData[VERTEX_COUNT] =
        {
            { { 0.0f, 0.0f } },
            { { 0.0f, 1.0f } },
            { { 1.0f, 1.0f } },
            { { 1.0f, 0.0f } },
        };
        constexpr uint32_t VERTEX_CIRCLE_COUNT = VERTEX_CIRCLE_END - VERTEX_CIRCLE_BEGIN;
        for(uint32_t i = 0; i < VERTEX_CIRCLE_COUNT; ++i)
        {
            float angle = 2 * Math::PI<float> * i / VERTEX_CIRCLE_COUNT;
            float x = Math::Cos(angle), y = Math::Sin(angle);
            vtxData[VERTEX_CIRCLE_BEGIN + i] = { { x, y } };
        }
        primVtxBuf_.InitializeHandle();
        primVtxBuf_.ReinitializeData(vtxData, uint32_t(ArraySize(vtxData)), GL_STATIC_DRAW);

        GLushort elemData[ELEM_COUNT] =
        {
            0, 1, 2, 0, 2, 3, // GL_TRIANGLES
            0, 1, 2, 3,       // GL_LINE_LOOP
            0, 2,             // GL_LINES,
            0, 3, 1,          // GL_TRIANGLES
        };
        for(uint32_t i = 0; i < VERTEX_CIRCLE_COUNT; ++i)
        {
            elemData[ELEM_LINE_CIRCLE_BEGIN + i] = VERTEX_CIRCLE_BEGIN + i;
            elemData[ELEM_FILL_CIRCLE_BEGIN + 3 * i + 0] = 0;
            elemData[ELEM_FILL_CIRCLE_BEGIN + 3 * i + 1] = VERTEX_CIRCLE_BEGIN + i;
            elemData[ELEM_FILL_CIRCLE_BEGIN + 3 * i + 2] = VERTEX_CIRCLE_BEGIN + (i + 1) % VERTEX_CIRCLE_COUNT;
        }
        primElemBuf_.InitializeHandle();
        primElemBuf_.ReinitializeData(elemData, uint32_t(ArraySize(elemData)), GL_STATIC_DRAW);
    }

    void InitializePrimitiveProgram()
    {
        const char *NATIVE_PRIMITIVE_PROGRAM_VS =
            R"____(
        #version 450 core
        uniform vec2 A;
        uniform vec2 B;
        in vec2 iPos;
        void main(void)
        {
            gl_Position = vec4(A * iPos + B, 0, 1);
        }
        )____";

        const char *TEXTURE_TRIANGLE_PROGRAM_VS =
            R"____(
        #version 450 core
        in vec2 iPos;
        in vec2 iTexCoord;
        out vec2 mTexCoord;
        void main(void)
        {
            gl_Position = vec4(iPos, 0, 1);
            mTexCoord   = iTexCoord;
        }
        )____";

        const char *NATIVE_TRIANGLE_PROGRAM_VS =
            R"____(
        #version 450 core
        uniform vec2 A;
        uniform vec2 B_A;
        uniform vec2 C_A;
        in vec2 iPos;
        void main(void)
        {
            gl_Position = vec4(A + iPos.x * B_A + iPos.y * C_A, 0, 1);
        }
        )____";

        const char *PURE_COLOR_FS =
            R"____(
        #version 450 core
        uniform vec4 FRAG_COLOR;
        void main(void)
        {
            gl_FragColor = FRAG_COLOR;
        }
        )____";

        const char *TEXTURE_FS =
            R"____(
        #version 450 core
        uniform sampler2D TEX;
        in vec2 mTexCoord;
        void main(void)
        {
            gl_FragColor = texture(TEX, mTexCoord);
        }
        )____";

        {
            affine_.prog = ProgramBuilder::BuildOnce(
                VertexShader::FromMemory(NATIVE_PRIMITIVE_PROGRAM_VS),
                FragmentShader::FromMemory(PURE_COLOR_FS));

            affine_.uniform_A          = affine_.prog.GetUniformVariable<Vec2f>("A");
            affine_.uniform_B          = affine_.prog.GetUniformVariable<Vec2f>("B");
            affine_.uniform_FRAG_COLOR = affine_.prog.GetUniformVariable<Vec4f>("FRAG_COLOR");

            auto iPos = affine_.prog.GetAttribVariable<Vec2f>("iPos");
            affine_.vao.InitializeHandle();
            affine_.vao.EnableAttrib(iPos);
            affine_.vao.BindVertexBufferToAttrib(iPos, primVtxBuf_, &PrimitiveVertex::pos, 0);
            affine_.vao.BindElementBuffer(primElemBuf_);
        }

        {
            triangle_.prog = ProgramBuilder::BuildOnce(
                VertexShader::FromMemory(NATIVE_TRIANGLE_PROGRAM_VS),
                FragmentShader::FromMemory(PURE_COLOR_FS));

            triangle_.uniform_A          = triangle_.prog.GetUniformVariable<Vec2f>("A");
            triangle_.uniform_B_A        = triangle_.prog.GetUniformVariable<Vec2f>("B_A");
            triangle_.uniform_C_A        = triangle_.prog.GetUniformVariable<Vec2f>("C_A");
            triangle_.uniform_FRAG_COLOR = triangle_.prog.GetUniformVariable<Vec4f>("FRAG_COLOR");

            auto iPos = triangle_.prog.GetAttribVariable<Vec2f>("iPos");
            triangle_.vao.InitializeHandle();
            triangle_.vao.EnableAttrib(iPos);
            triangle_.vao.BindVertexBufferToAttrib(iPos, primVtxBuf_, &PrimitiveVertex::pos, 0);
            triangle_.vao.BindElementBuffer(primElemBuf_);
        }

        {
            texTriangle_.prog_ = ProgramBuilder::BuildOnce(
                VertexShader::FromMemory(TEXTURE_TRIANGLE_PROGRAM_VS),
                FragmentShader::FromMemory(TEXTURE_FS));

            texTriangle_.uniform_TEX = texTriangle_.prog_.GetUniformVariable<Texture2DUnit>("TEX");

            texTriangle_.attrib_iPos      = texTriangle_.prog_.GetAttribVariable<Vec2f>("iPos");
            texTriangle_.attrib_iTexCoord = texTriangle_.prog_.GetAttribVariable<Vec2f>("iTexCoord");

            texTriangle_.vao.InitializeHandle();
            texTriangle_.vao.EnableAttrib(texTriangle_.attrib_iPos);
            texTriangle_.vao.EnableAttrib(texTriangle_.attrib_iTexCoord);
        }
    }

public:

    struct TexturedVertex
    {
        Vec2f pos;
        Vec2f tex;
    };

    /**
     * @param pixelSize 屏幕宽高
     */
    void Initialize(const Vec2f &pixelSize)
    {
        pixelSize_ = pixelSize;

        InitializePrimitiveBuffer();
        InitializePrimitiveProgram();
    }

    /**
     * @brief 重新设置屏幕宽高
     */
    void Resize(const Vec2f &pixelSize) noexcept
    {
        pixelSize_ = pixelSize;
    }

    /**
     * @brief 在屏幕上绘制指定颜色的线段
     * 
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawLine(const Vec2f &p1, const Vec2f &p2, const Vec4f &color) const
    {
        affine_.prog.Bind();
        affine_.vao.Bind();

        affine_.uniform_A.BindValue(p2 - p1);
        affine_.uniform_B.BindValue(p1);
        affine_.uniform_FRAG_COLOR.BindValue(color);

        RenderContext::DrawElements(
            GL_LINES, ELEM_LINE_BEGIN, ELEM_LINE_END - ELEM_LINE_BEGIN, primElemBuf_.GetElemType());

        affine_.vao.Unbind();
        affine_.prog.Unbind();
    }

    /**
     * @brief 在屏幕上绘制指定颜色的线段
     * 
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawLineP(const Vec2f &p1, const Vec2f &p2, const Vec4f &color) const
    {
        Vec2f tp1(2 * p1.x / pixelSize_.x - 1, 1 - 2 * p1.y / pixelSize_.y);
        Vec2f tp2(2 * p2.x / pixelSize_.x - 1, 1 - 2 * p2.y / pixelSize_.y);
        DrawLine(tp1, tp2, color);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的矩形
     * 
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawQuad(const Vec2f &LB, const Vec2f &RT, const Vec4f &color, bool fill = true) const
    {
        affine_.prog.Bind();
        affine_.vao.Bind();
        affine_.uniform_A.BindValue(RT - LB);
        affine_.uniform_B.BindValue(LB);
        affine_.uniform_FRAG_COLOR.BindValue(color);

        if(fill)
        {
            RenderContext::DrawElements(
                GL_TRIANGLES, ELEM_FILL_QUAD_BEGIN, ELEM_FILL_QUAD_END - ELEM_FILL_QUAD_BEGIN, primElemBuf_.GetElemType());
        }
        else
        {
            RenderContext::DrawElements(
                GL_LINE_LOOP, ELEM_LINE_QUAD_BEGIN, ELEM_LINE_QUAD_END - ELEM_LINE_QUAD_BEGIN, primElemBuf_.GetElemType());
        }

        affine_.vao.Unbind();
        affine_.prog.Unbind();
    }

    /**
     * @brief 在屏幕上绘制指定颜色的矩形
     * 
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawQuadP(const Vec2f &LT, const Vec2f &RB, const Vec4f &color, bool fill = true) const
    {
        Vec2f tLT(2 * LT.x / pixelSize_.x - 1, 1 - 2 * LT.y / pixelSize_.y);
        Vec2f tRB(2 * RB.x / pixelSize_.x - 1, 1 - 2 * RB.y / pixelSize_.y);
        DrawQuad(tLT, tRB, color, fill);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的三角形
     * 
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawTriangle(const Vec2f &A, const Vec2f &B, const Vec2f &C, const Vec4f &color, bool fill = true) const
    {
        GLenum oldFill = RenderContext::GetFillMode();

        triangle_.prog.Bind();
        triangle_.vao.Bind();
        triangle_.uniform_A.BindValue(A);
        triangle_.uniform_B_A.BindValue(B - A);
        triangle_.uniform_C_A.BindValue(C - A);
        triangle_.uniform_FRAG_COLOR.BindValue(color);

        RenderContext::SetFillMode(fill ? GL_FILL : GL_LINE);
        RenderContext::DrawElements(
            GL_TRIANGLES, ELEM_TRIANGLE_BEGIN, ELEM_TRIANGLE_END - ELEM_TRIANGLE_BEGIN, primElemBuf_.GetElemType());

        triangle_.vao.Unbind();
        triangle_.prog.Unbind();

        RenderContext::SetFillMode(oldFill);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的三角形
     * 
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawTriangleP(const Vec2f &A, const Vec2f &B, const Vec2f &C, const Vec4f &color, bool fill = true) const
    {
        Vec2f sA(2 * A.x / pixelSize_.x - 1, 1 - 2 * A.y / pixelSize_.y);
        Vec2f sB(2 * B.x / pixelSize_.x - 1, 1 - 2 * B.y / pixelSize_.y);
        Vec2f sC(2 * C.x / pixelSize_.x - 1, 1 - 2 * C.y / pixelSize_.y);
        DrawTriangle(sA, sB, sC, color, fill);
    }

    /**
     * @brief 在屏幕上绘制一组具有指定纹理的三角形
     * 
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawTexturedTriangles(const TexturedVertex *vtx, uint32_t vtxCount, const Texture2D &tex)
    {
        AGZ_ASSERT(vtx && vtxCount && vtxCount % 3 == 0);

        VertexBuffer<TexturedVertex> vtxBuf(true);
        vtxBuf.ReinitializeData(vtx, vtxCount, GL_STATIC_DRAW);

        texTriangle_.vao.BindVertexBufferToAttrib(texTriangle_.attrib_iPos, vtxBuf, &TexturedVertex::pos, 0);
        texTriangle_.vao.BindVertexBufferToAttrib(texTriangle_.attrib_iTexCoord, vtxBuf, &TexturedVertex::tex, 1);

        texTriangle_.prog_.Bind();
        texTriangle_.vao.Bind();
        tex.Bind(0);
        texTriangle_.uniform_TEX.BindValue({ 0 });

        RenderContext::DrawVertices(GL_TRIANGLES, 0, vtxCount);

        tex.Unbind(0);
        texTriangle_.vao.Unbind();
        texTriangle_.prog_.Unbind();
    }

    /**
     * @brief 在屏幕上绘制个具有指定纹理的矩形
     *
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawTexturedQuad(const Vec2f &LB, const Vec2f &RT, const Texture2D &tex)
    {
        const TexturedVertex vtxData[] =
        {
            { { LB.x, LB.y }, { 0, 1 } },
            { { LB.x, RT.y }, { 0, 0 } },
            { { RT.x, RT.y }, { 1, 0 } },
            { { LB.x, LB.y }, { 0, 1 } },
            { { RT.x, RT.y }, { 1, 0 } },
            { { RT.x, LB.y }, { 1, 1 } },
        };
        DrawTexturedTriangles(vtxData, 6, tex);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的圆
     * 
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawCircle(const Vec2f &centre, const Vec2f &size, const Vec4f &color, bool fill = true) const
    {
        GLenum oldFill = RenderContext::GetFillMode();

        affine_.prog.Bind();
        affine_.vao.Bind();

        affine_.uniform_A.BindValue(size);
        affine_.uniform_B.BindValue(centre);
        affine_.uniform_FRAG_COLOR.BindValue(color);

        if(fill)
        {
            RenderContext::SetFillMode(GL_FILL);
            RenderContext::DrawElements(
                GL_TRIANGLES, ELEM_FILL_CIRCLE_BEGIN, ELEM_FILL_CIRCLE_END - ELEM_FILL_CIRCLE_BEGIN, primElemBuf_.GetElemType());
        }
        else
        {
            RenderContext::SetFillMode(GL_LINE);
            RenderContext::DrawElements(
                GL_LINE_LOOP, ELEM_LINE_CIRCLE_BEGIN, ELEM_LINE_CIRCLE_END - ELEM_LINE_CIRCLE_BEGIN, primElemBuf_.GetElemType());
        }

        affine_.vao.Unbind();
        affine_.prog.Unbind();

        RenderContext::SetFillMode(oldFill);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的圆
     * 
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawCircleP(const Vec2f &centre, const Vec2f &size, const Vec2f &C, const Vec4f &color, bool fill = true) const
    {
        Vec2f sCentre(2 * centre.x / pixelSize_.x - 1, 1 - 2 * centre.y / pixelSize_.y);
        Vec2f sSize(2 * size / pixelSize_);
        DrawCircle(sCentre, sSize, color, fill);
    }
};

/**
 * @brief 用于直接在屏幕上绘制三维图形的工具
 */
class Immediate3D
{
public:

    /**
     * @brief 初始化绘制器
     * 
     * 在绘制前必须调用且只能调用一次
     */
    void Initialize()
    {
        InitializeProgram();
        InitializeVAO();
    }

    /**
     * @brief 绘制一条纯色线段，顶点数据为(0, 0, 0)-(1, 0, 0)
     * @param transform 顶点变换矩阵
     * @param color 绘制颜色
     */
    void DrawLine(const Mat4f &transform, const Vec4f &color)
    {
        pureColor_.prog.Bind();
        pureColor_.vao.Bind();

        pureColor_.uMAT.BindValue(transform);
        pureColor_.uCOLOR.BindValue(color);

        RenderContext::DrawVertices(GL_LINES, 0, 2);

        pureColor_.vao.Unbind();
        pureColor_.prog.Unbind();
    }

    /**
     * @brief 绘制一个纯色圆环，顶点平铺在xOz平面上，圆心为远点，半径为1
     * @param transform 顶点变换矩阵
     * @param color 绘制颜色
     */
    void DrawCircle(const Mat4f &transform, const Vec4f &color)
    {
        pureColor_.prog.Bind();
        pureColor_.vao.Bind();

        pureColor_.uMAT.BindValue(transform);
        pureColor_.uCOLOR.BindValue(color);

        RenderContext::DrawVertices(GL_LINE_LOOP, 2, CIRCLE_VERTEX_COUNT);

        pureColor_.vao.Unbind();
        pureColor_.prog.Unbind();
    }

private:

    struct
    {
        Program prog;

        UniformVariable<Mat4f> uMAT;
        UniformVariable<Vec4f> uCOLOR;

        AttribVariable<Vec3f> aPos;

        VertexArray vao;
        VertexBuffer<Vec3f> vtxBuf;
    } pureColor_;

    void InitializeProgram()
    {
        static const char *VS = R"____(
        #version 450 core
        uniform mat4 WVP;
        in vec3 iPos;
        void main(void)
        {
            gl_Position = WVP * vec4(iPos, 1);
        }
        )____";

        static const char *FS = R"____(
        #version 450 core
        uniform vec4 COLOR;
        out vec4 fragColor;
        void main(void)
        {
            fragColor = COLOR;
        }
        )____";

        pureColor_.prog = ProgramBuilder::BuildOnce(
            VertexShader::FromMemory(VS), FragmentShader::FromMemory(FS));
        pureColor_.uMAT = pureColor_.prog.GetUniformVariable<Mat4f>("WVP");
        pureColor_.uCOLOR = pureColor_.prog.GetUniformVariable<Vec4f>("COLOR");

        pureColor_.aPos = pureColor_.prog.GetAttribVariable<Vec3f>("iPos");
    }

    static constexpr uint32_t CIRCLE_VERTEX_COUNT = 64;

    static constexpr uint32_t VERTEX_LINE_BEG = 0;
    static constexpr uint32_t VERTEX_LINE_END = VERTEX_LINE_BEG + 2;
    static constexpr uint32_t VERTEX_CIRCLE_BEG = VERTEX_LINE_END;
    static constexpr uint32_t VERTEX_CIRCLE_END = VERTEX_CIRCLE_BEG + CIRCLE_VERTEX_COUNT;

    void InitializeVAO()
    {
        pureColor_.vao.InitializeHandle();
        pureColor_.vao.EnableAttrib(pureColor_.aPos);

        static Vec3f vtxData[2 + CIRCLE_VERTEX_COUNT] =
        {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
        };
        for(uint32_t i = 0; i < CIRCLE_VERTEX_COUNT; ++i)
        {
            Deg angle = Deg(360.0f * i / CIRCLE_VERTEX_COUNT);
            vtxData[i + 2].x = Cos(angle);
            vtxData[i + 2].y = 0;
            vtxData[i + 2].z = Sin(angle);
        }

        pureColor_.vtxBuf.InitializeHandle();
        pureColor_.vtxBuf.ReinitializeData(vtxData, static_cast<uint32_t>(ArraySize(vtxData)), GL_STATIC_DRAW);

        pureColor_.vao.BindVertexBufferToAttrib(pureColor_.aPos, pureColor_.vtxBuf, 0);
    }
};

} // namespace AGZ::GraphicsAPI::GL
