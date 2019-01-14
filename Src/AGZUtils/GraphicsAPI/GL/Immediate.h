#pragma once

#include "Buffer.h"
#include "Program.h"
#include "ProgramBuilder.h"
#include "VertexArray.h"

namespace AGZ::GL
{
    
/**
 * @brief 用于直接在屏幕上绘制内容的工具
 */
class Immediate : public Uncopiable
{
    struct PrimitiveVertex
    {
        Vec2f pos;
    };
    VertexBuffer<PrimitiveVertex> primVtxBuf_;
    ElementBuffer<GLubyte> primElemBuf_;

    Program primProg_;
    UniformVariable<Vec2f> uniform_A;
    UniformVariable<Vec2f> uniform_B;
    UniformVariable<Vec4f> uniform_FRAG_COLOR;

    VertexArray primVAO_;

    Vec2f pixelSize_;

    void InitializePrimitiveBuffer()
    {
        /*
            0: (0, 0)
            1: (0, 1)
            2: (1, 1)
            3: (1, 0)
        */
        PrimitiveVertex vtxData[] =
        {
            { { 0.0f, 0.0f } },
            { { 0.0f, 1.0f } },
            { { 1.0f, 1.0f } },
            { { 1.0f, 0.0f } },
        };
        primVtxBuf_.InitializeHandle();
        primVtxBuf_.ReinitializeData(vtxData, ArraySize(vtxData), GL_STATIC_DRAW);

        /*
            0-5: Quad
            6-7: Line
        */
        GLubyte elemData[] =
        {
            0, 1, 2, 0, 2, 3,
            0, 2,
        };
        primElemBuf_.InitializeHandle();
        primElemBuf_.ReinitializeData(elemData, ArraySize(elemData), GL_STATIC_DRAW);
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

        const char *NATIVE_PRIMITIVE_PROGRAM_FS =
            R"____(
        #version 450 core
        uniform vec4 FRAG_COLOR;
        out vec4 fragColor;
        void main(void)
        {
            fragColor = FRAG_COLOR;
        }
        )____";

        primProg_ = ProgramBuilder::BuildOnce(
            VertexShader  ::FromMemory(NATIVE_PRIMITIVE_PROGRAM_VS),
            FragmentShader::FromMemory(NATIVE_PRIMITIVE_PROGRAM_FS));

        uniform_A          = primProg_.GetUniformVariable<Vec2f>("A");
        uniform_B          = primProg_.GetUniformVariable<Vec2f>("B");
        uniform_FRAG_COLOR = primProg_.GetUniformVariable<Vec4f>("FRAG_COLOR");
    }

    void InitializePrimitiveVAO()
    {
        auto iPos = primProg_.GetAttribVariable<Vec2f>("iPos");

        primVAO_.InitializeHandle();
        primVAO_.EnableAttrib(iPos);
        primVAO_.BindVertexBufferToAttrib(iPos, primVtxBuf_, &PrimitiveVertex::pos, 0);
        primVAO_.BindElementBuffer(primElemBuf_);
    }

public:

    /**
     * @param pixelSize 屏幕宽高
     */
    void Initialize(const Vec2f &pixelSize)
    {
        pixelSize_ = pixelSize;

        InitializePrimitiveBuffer();
        InitializePrimitiveProgram();
        InitializePrimitiveVAO();
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
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawLine(const Vec2f &p1, const Vec2f &p2, const Vec4f &color) const
    {
        primProg_.Bind();
        primVAO_.Bind();

        uniform_A.BindValue(p2 - p1);
        uniform_B.BindValue(p1);
        uniform_FRAG_COLOR.BindValue(color);

        glDrawElements(GL_LINES, 2, primElemBuf_.GetElemType(), (decltype(primElemBuf_)::ElemType*)(nullptr) + 6);

        primVAO_.Unbind();
        primProg_.Unbind();
    }

    /**
     * @brief 在屏幕上绘制指定颜色的线段
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawLineP(const Vec2f &p1, const Vec2f &p2, const Vec4f &color) const
    {
        Vec2f tp1(2 * p1.x - 1, 1 - 2 * p1.y);
        Vec2f tp2(2 * p2.x - 1, 1 - 2 * p2.y);
        DrawLine(tp1 ,tp2, color);
    }

    /**
     * @brief 在屏幕上绘制指定颜色的矩形
     * 以屏幕中心为原点，坐标范围[-1, 1]^2
     */
    void DrawQuad(const Vec2f &LB, const Vec2f &RT, const Vec4f &color) const
    {
        primProg_.Bind();
        primVAO_.Bind();

        uniform_A.BindValue(RT - LB);
        uniform_B.BindValue(LB);
        uniform_FRAG_COLOR.BindValue(color);

        glDrawElements(GL_TRIANGLES, 6, primElemBuf_.GetElemType(), nullptr);

        primVAO_.Unbind();
        primProg_.Unbind();
    }

    /**
     * @brief 在屏幕上绘制指定颜色的矩形
     * 以屏幕左上角为原点，坐标单位为像素
     */
    void DrawQuadP(const Vec2f &LT, const Vec2f &RB, const Vec4f &color) const
    {
        Vec2f tLT(2 * LT.x - 1, 1 - 2 * LT.y);
        Vec2f tRB(2 * RB.x - 1, 1 - 2 * RB.y);
        DrawQuad(tLT, tRB, color);
    }
};

} // namespace AGZ::GL
