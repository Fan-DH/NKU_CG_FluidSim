/**
 * Lagrangian2dComponent.cpp: 2D 拉格朗日流体组件实现文件
 * 负责粒子系统的初始化、步进仿真，以及与渲染器之间的对接
 */

#include "Lagrangian2dComponent.h"
#include "Global.h"

#include <algorithm>

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        /**
         * 关闭组件并释放相关资源
         * 包括渲染器 Renderer、求解器 Solver 和粒子系统 ParticleSystem2d
         */
        void Lagrangian2dComponent::shutDown()
        {
            delete renderer, solver, ps;
            renderer = NULL;
            solver = NULL;
            ps = NULL;
        }

        /**
         * 初始化组件
         * 创建渲染器和粒子系统，根据参数生成初始粒子并构造求解器
         */
        void Lagrangian2dComponent::init()
        {
            if (renderer != NULL || solver != NULL || ps != NULL)
            {
                shutDown();
            }

            // 清空计时器，避免上一次仿真残留的统计数据干扰
            Glb::Timer::getInstance().clear();

            // 创建并初始化渲染器
            renderer = new Renderer();
            renderer->init();

            // 创建粒子系统，并设置容器边界为 [-1, -1] ~ [1, 1]
            ps = new ParticleSystem2d();
            ps->setContainerSize(glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, 1.0f));
            if (Lagrangian2dPara::enableFountain2d)
            {
                // 喷泉模式下不预先生成流体块，只初始化空间哈希结构
                ps->updateBlockInfo();
                Glb::Logger::getInstance().addLog("2d Particle System initialized. particle num: " + std::to_string(ps->particles.size()) + " (fountain mode)");
            }
            else
            {
                // 普通模式：根据配置的多个 fluidBlocks 生成初始粒子
                for (int i = 0; i < Lagrangian2dPara::fluidBlocks.size(); i++)
                {
                    ps->addFluidBlock(
                        Lagrangian2dPara::fluidBlocks[i].lowerCorner,
                        Lagrangian2dPara::fluidBlocks[i].upperCorner,
                        Lagrangian2dPara::fluidBlocks[i].initVel,
                        Lagrangian2dPara::fluidBlocks[i].particleSpace);
                }
            }
            ps->updateBlockInfo();
            Glb::Logger::getInstance().addLog("2d Particle System initialized. particle num: " + std::to_string(ps->particles.size()));

            // 基于粒子系统创建求解器
            solver = new Solver(*ps);
        }

        /**
         * 执行一次 2D 拉格朗日流体仿真
         * 普通模式：对固定数量的粒子做多子步积分
         * 喷泉模式：持续从喷口发射粒子，并删除落入排水口或离开模拟区域的粒子
         */
        void Lagrangian2dComponent::simulate()
        {
            if (ps == NULL || solver == NULL)
            {
                return;
            }

            if (!Lagrangian2dPara::enableFountain2d)
            {
                // 普通模式：每帧执行 substep 次求解，以提高数值稳定性
                for (int i = 0; i < Lagrangian2dPara::substep; i++)
                {
                    ps->updateBlockInfo();
                    solver->solve();
                }
                return;
            }

            const float drainMinX = -0.25f;
            const float drainMaxX = 0.25f;
            const float drainY = -1.0f;

            const float emitterMinX = -0.03f;
            const float emitterMaxX = 0.03f;
            const float emitterY = -0.95f;
            const float emitterSpeed = 13.0f;
            const float emitterHalfAngleRad = 0.10f;

            // 坐标缩放因子：粒子系统内部使用缩放后的坐标
            const float scale = ps->scale;
            const float invScale = 1.0f / scale;

            // 粒子最大数量和模拟区域外扩边界
            const size_t maxParticleCount = 40000;
            const float killBound = 1.25f;

            Glb::RandomGenerator rand;
            static uint64_t emitterStep = 0;
            emitterStep++;

            for (int i = 0; i < Lagrangian2dPara::substep; i++)
            {
                if (ps->particles.size() < maxParticleCount)
                {
                    // 以粒子直径为单位设置喷射间距和速度范围
                    const float particleSpace = ps->particleDiameter;
                    const float tangent = (std::tan)(emitterHalfAngleRad);
                    const float maxVx = tangent * emitterSpeed;

                    const bool shouldEmitThisStep = true;
                    if (shouldEmitThisStep)
                    {
                        // 为喷口发射位置加入少量随机扰动，避免规则网格导致的条纹
                        const float jitterX = (rand.GetUniformRandom() - 0.5f) * particleSpace * 0.05f;
                        const float phaseSpacingY = particleSpace * 0.04f;
                        const float baseY = emitterY + (static_cast<float>(emitterStep % 4u) - 1.5f) * phaseSpacingY;

                        // 分多层发射粒子，层间错列排布，形成更自然的喷流截面
                        const int layers = 2;
                        const float layerSpacing = particleSpace * 0.85f;
                        const float xStep = particleSpace * 0.85f;
                        for (int layer = 0; layer < layers; layer++)
                        {
                            const float y = baseY + static_cast<float>(layer) * layerSpacing;
                            const float layerXOffset = (layer & 1) ? (0.5f * xStep) : 0.0f;
                            for (float x = emitterMinX + layerXOffset; x <= emitterMaxX + 1e-6f; x += xStep)
                            {
                                if (ps->particles.size() >= maxParticleCount)
                                {
                                    break;
                                }

                                ParticleInfo2d p{};
                                p.position = glm::vec2(x + jitterX, y) * scale;
                                // 在 [-maxVx, maxVx] 范围内随机选择水平速度，形成扇形喷射
                                p.velocity = glm::vec2(LERP(-0.7f * maxVx, 0.7f * maxVx, rand.GetUniformRandom()), emitterSpeed);
                                p.density = Lagrangian2dPara::density;
                                p.blockId = ps->getBlockIdByPosition(p.position);
                                if (p.blockId != static_cast<uint32_t>(-1))
                                {
                                    ps->particles.push_back(p);
                                }
                            }
                        }
                    }
                }

                // 先更新空间哈希信息，再进行一次求解
                ps->updateBlockInfo();
                solver->solve();

                // 删除落入排水口或飞出模拟区域的粒子
                ps->particles.erase(
                    std::remove_if(
                        ps->particles.begin(),
                        ps->particles.end(),
                        [&](const ParticleInfo2d &p)
                        {
                            const glm::vec2 wp = p.position * invScale;
                            const bool inDrain = (wp.x >= drainMinX && wp.x <= drainMaxX && wp.y <= drainY);
                            const bool outOfSim = (wp.x < -killBound || wp.x > killBound || wp.y < -killBound || wp.y > killBound);
                            return inDrain || outOfSim;
                        }),
                    ps->particles.end());
            }
        }

        /**
         * 获取当前 2D 拉格朗日流体场的离屏渲染结果纹理
         * 在 UI 层可以将该纹理绘制到一个全屏四边形上进行展示
         */
        GLuint Lagrangian2dComponent::getRenderedTexture()
        {
            renderer->draw(*ps);
            return renderer->getRenderedTexture();
        }
    }
}
