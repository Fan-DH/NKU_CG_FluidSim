#include "ParticleSystem2d.h"
#include <iostream>
#include "Global.h"
#include <unordered_set>

namespace FluidSimulation
{

    namespace Lagrangian2d
    {
        ParticleSystem2d::ParticleSystem2d()
        {
        }

        ParticleSystem2d::~ParticleSystem2d()
        {
        }

        // 设置流体容器的大小
        void ParticleSystem2d::setContainerSize(glm::vec2 lower = glm::vec2(-1.0f, -1.0f), glm::vec2 upper = glm::vec2(1.0f, 1.0f))
        {
            // 应用缩放
            lower *=  Lagrangian2dPara::scale;
            upper *= Lagrangian2dPara::scale;

            // 设置边界,考虑支持半径和粒子直径
            lowerBound = lower - supportRadius + particleDiameter;
            upperBound = upper + supportRadius - particleDiameter;
            containerCenter = (lowerBound + upperBound) / 2.0f;

            glm::vec2 size = upperBound - lowerBound;

            // 计算块的数量和大小
            blockNum.x = floor(size.x / supportRadius);
            blockNum.y = floor(size.y / supportRadius);
            blockSize = glm::vec2(size.x / blockNum.x, size.y / blockNum.y);

            // 初始化块偏移数组
            blockIdOffs.resize(9);
            int p = 0;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    blockIdOffs[p] = blockNum.x * j + i;
                    p++;
                }
            }

            // 清空粒子数组
            particles.clear();
        }

        // 添加流体块
        int ParticleSystem2d::addFluidBlock(glm::vec2 lowerCorner, glm::vec2 upperCorner, glm::vec2 v0, float particleSpace)
        {
            // 应用缩放
            lowerCorner *= Lagrangian2dPara::scale;
            upperCorner *= Lagrangian2dPara::scale;

            glm::vec2 size = upperCorner - lowerCorner;

            // 检查边界
            if (lowerCorner.x < lowerBound.x ||
                lowerCorner.y < lowerBound.y ||
                upperCorner.x > upperBound.x ||
                upperCorner.y > upperBound.y)
            {
                return 0;
            }

            // 计算粒子数量
            glm::uvec2 particleNum = glm::uvec2(size.x / particleSpace, size.y / particleSpace);
            std::vector<ParticleInfo2d> tempParticles(particleNum.x * particleNum.y);

            // 随机生成器,用于粒子位置的扰动
            Glb::RandomGenerator rand;
            int p = 0;
            // 在流体块中生成粒子
            for (int idX = 0; idX < particleNum.x; idX++)
            {
                for (int idY = 0; idY < particleNum.y; idY++)
                {
                    // 添加随机扰动避免规则排列
                    float x = (idX + rand.GetUniformRandom()) * particleSpace;
                    float y = (idY + rand.GetUniformRandom()) * particleSpace;

                    // 设置粒子属性
                    tempParticles[p].position = lowerCorner + glm::vec2(x, y);
                    tempParticles[p].blockId = getBlockIdByPosition(tempParticles[p].position);
                    tempParticles[p].density = Lagrangian2dPara::density;
                    tempParticles[p].velocity = v0;
                    p++;
                }
            }

            // 将新粒子添加到系统中
            particles.insert(particles.end(), tempParticles.begin(), tempParticles.end());
            return particles.size();
        }

        // 根据位置获取块ID
        uint32_t ParticleSystem2d::getBlockIdByPosition(glm::vec2 position)
        {
            // 检查边界
            if (position.x < lowerBound.x ||
                position.y < lowerBound.y ||
                position.x > upperBound.x ||
                position.y > upperBound.y)
            {
                return -1;
            }

            // 计算块索引
            glm::vec2 deltePos = position - lowerBound;
            uint32_t c = floor(deltePos.x / blockSize.x);
            uint32_t r = floor(deltePos.y / blockSize.y);
            return r * blockNum.x + c;
        }

        // 更新块信息
        void ParticleSystem2d::updateBlockInfo()
        {
            // 根据块ID对粒子进行排序
            std::sort(particles.begin(), particles.end(),
                      [=](ParticleInfo2d &first, ParticleInfo2d &second)
                      {
                          return first.blockId < second.blockId;
                      });

            // 更新每个块中粒子的范围
            blockExtens = std::vector<glm::uvec2>(blockNum.x * blockNum.y, glm::uvec2(0, 0));
            int curBlockId = 0;
            int left = 0;
            int right;
            for (right = 0; right < particles.size(); right++)
            {
                if (particles[right].blockId != curBlockId)
                {
                    blockExtens[curBlockId] = glm::uvec2(left, right);
                    left = right;
                    curBlockId = particles[right].blockId;
                }
            }
            blockExtens[curBlockId] = glm::uvec2(left, right);
        }
    }
}
