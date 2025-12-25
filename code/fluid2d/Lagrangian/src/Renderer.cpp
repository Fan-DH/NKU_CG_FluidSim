#include "Lagrangian/include/Renderer.h"

#include <iostream>
#include <fstream>
#include "Configure.h"

// 本文件实现 2D 拉格朗日粒子流体的渲染逻辑，封装了 OpenGL 调用
// 阅读本文件前，建议对 VAO/VBO、FBO、纹理等 OpenGL 基础概念有一定了解

namespace FluidSimulation
{

    namespace Lagrangian2d
    {

        Renderer::Renderer()
        {
        }

        // 初始化渲染相关资源：着色器、VAO/VBO、FBO、纹理和深度模板缓冲
        int32_t Renderer::init()
        {
            extern std::string shaderPath;

            // 加载粒子渲染顶点/片元着色器并构建着色器程序
            std::string particleVertShaderPath = shaderPath + "/DrawParticles2d.vert";
            std::string particleFragShaderPath = shaderPath + "/DrawParticles2d.frag";
            shader = new Glb::Shader();
            shader->buildFromFile(particleVertShaderPath, particleFragShaderPath);

            // 生成顶点数组对象（VAO），用于记录顶点属性配置
            glGenVertexArrays(1, &VAO);
            // 生成存储粒子位置的顶点缓冲对象（VBO）
            glGenBuffers(1, &positionVBO);
            // 生成存储粒子密度的顶点缓冲对象（VBO）
            glGenBuffers(1, &densityVBO);
            // 生成存储标记（是否属于排水口等）的顶点缓冲对象（VBO）
            glGenBuffers(1, &markVBO);

            // 生成离屏帧缓冲对象（FBO），用于渲染到纹理
            glGenFramebuffers(1, &FBO);
            // 绑定帧缓冲，后续的绘制输出到此 FBO
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 创建颜色纹理，作为 FBO 的颜色附件，用来保存渲染结果
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

            // 将颜色纹理附着到帧缓冲的颜色附件上
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            // 创建深度/模板渲染缓冲对象（RBO），用于深度测试和模板测试
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // 将 RBO 附着到帧缓冲的深度/模板附件
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 设置视口大小，使其与渲染分辨率一致
            glViewport(0, 0, imageWidth, imageHeight);

            return 0;
        }

        // 根据粒子系统数据绘制当前帧
        // 会将粒子位置、密度和标记上传到 GPU，然后在离屏 FBO 中绘制所有粒子
        void Renderer::draw(ParticleSystem2d& ps)
        {
            // 是否处于喷泉模式，决定是否绘制排水口标记及发射粒子辅助标记
            const bool fountain = Lagrangian2dPara::enableFountain2d;

            // 排水口所在的世界坐标区域（喷泉模式下用于高亮显示）
            const float drainMinX = -0.25f;
            const float drainMaxX = 0.25f;
            const float drainY = -1.0f;
            const float highlightThickness = 0.02f;

            // 排水口边缘的辅助标记点数量
            const size_t markerCount = fountain ? 32 : 0;

            std::vector<glm::vec2> positions;
            std::vector<float> densities;
            std::vector<float> marks;
            positions.reserve(ps.particles.size() + markerCount);
            densities.reserve(ps.particles.size() + markerCount);
            marks.reserve(ps.particles.size() + markerCount);

            const float invScale = 1.0f / ps.scale;
            for (const ParticleInfo2d &p : ps.particles)
            {
                // 直接使用粒子世界坐标（已包含 scale），在着色器中再根据 scale 做归一化
                positions.push_back(p.position);
                densities.push_back(p.density);

                float markValue = 0.0f;
                if (fountain)
                {
                    // 将位置转换到未缩放的世界坐标，用于与排水口区域比较
                    const glm::vec2 wp = p.position * invScale;
                    const bool inDrainRegion = (wp.x >= drainMinX && wp.x <= drainMaxX && wp.y <= drainY + highlightThickness);
                    if (inDrainRegion)
                    {
                        // 标记排水口附近的粒子，在着色器中可以用不同颜色显示
                        markValue = 1.0f;
                    }
                }
                marks.push_back(markValue);
            }

            if (fountain)
            {
                // 在排水口边缘补充一圈静态标记点，用于强化排水口轮廓
                for (size_t i = 0; i < markerCount; i++)
                {
                    const float t = markerCount <= 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(markerCount - 1);
                    const float x = LERP(drainMinX, drainMaxX, t);
                    positions.push_back(glm::vec2(x, drainY) * ps.scale);
                    densities.push_back(0.0f);
                    marks.push_back(1.0f);
                }
            }

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2), positions.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, densityVBO);
            glBufferData(GL_ARRAY_BUFFER, densities.size() * sizeof(float), densities.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, markVBO);
            glBufferData(GL_ARRAY_BUFFER, marks.size() * sizeof(float), marks.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);

            particleNum = positions.size();

            // 在离屏帧缓冲中开始渲染
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 清空颜色和深度缓冲区，背景设为白色
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 准备绘制：绑定 VAO 和使用粒子渲染着色器
            glBindVertexArray(VAO);
            shader->use();
            // 将缩放系数传入着色器，用于坐标归一化
            shader->setFloat("scale", ps.scale);

            // 使用可编程点大小，由着色器决定每个粒子的显示大小
            glEnable(GL_PROGRAM_POINT_SIZE);

            // 绘制所有粒子（点精灵）
            glDrawArrays(GL_POINTS, 0, particleNum);

            // 解绑 FBO，恢复默认帧缓冲
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // 获取离屏渲染结果对应的颜色纹理 ID，用于在外部窗口中显示
        GLuint Renderer::getRenderedTexture()
        {
            return textureID;
        }
    }

}
