/**
 * Solver.cpp: 2D 拉格朗日流体求解器实现文件
 * 基于 SPH 的 2D 粒子流体单步仿真：密度、压力、压力力、粘性力、重力、显式 Euler、边界处理、空间哈希更新
 */

#include "Lagrangian/include/Solver.h"
#include "Global.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        Solver::Solver(ParticleSystem2d &ps) : mPs(ps)
        {
        }

        static inline float clampFloat(float value, float low, float high)
        {
            return (std::max)(low, (std::min)(high, value));
        }

        static inline glm::vec2 clampVec2(const glm::vec2 &value, const glm::vec2 &low, const glm::vec2 &high)
        {
            return glm::vec2(
                clampFloat(value.x, low.x, high.x),
                clampFloat(value.y, low.y, high.y));
        }

        static inline bool isValidBlockId(uint32_t blockId, uint32_t blockCount)
        {
            return blockId < blockCount;
        }

        static inline glm::uvec2 blockCoordFromId(uint32_t blockId, uint32_t blocksX)
        {
            return glm::uvec2(blockId % blocksX, blockId / blocksX);
        }

        static inline uint32_t blockIdFromCoord(const glm::uvec2 &coord, uint32_t blocksX)
        {
            return coord.y * blocksX + coord.x;
        }

        static inline float poly6Kernel2D(float r2, float h2, float poly6Coeff)
        {
            if (r2 >= h2)
            {
                return 0.0f;
            }

            const float diff = h2 - r2;
            const float diff3 = diff * diff * diff;
            return poly6Coeff * diff3;
        }

        static inline glm::vec2 spikyGradKernel2D(const glm::vec2 &r, float dist, float h, float spikyGradCoeff, float eps)
        {
            if (dist <= eps || dist >= h)
            {
                return glm::vec2(0.0f);
            }

            const float diff = h - dist;
            const float diff2 = diff * diff;
            const glm::vec2 dir = r / dist;
            return spikyGradCoeff * diff2 * dir;
        }

        static inline float viscosityLaplacian2D(float dist, float h, float viscLapCoeff)
        {
            if (dist >= h)
            {
                return 0.0f;
            }
            return viscLapCoeff * (h - dist);
        }

        void Solver::solve()
        {
            if (mPs.particles.empty())
            {
                return;
            }

            const int32_t substep = (std::max)(1, Lagrangian2dPara::substep);
            const float dt = Lagrangian2dPara::dt / static_cast<float>(substep);

            const float restDensity = Lagrangian2dPara::density;
            const float stiffness = Lagrangian2dPara::stiffness;
            const float exponent = Lagrangian2dPara::exponent;
            const float viscosity = Lagrangian2dPara::viscosity;
            const float maxVelocity = Lagrangian2dPara::maxVelocity;
            const float velocityAttenuation = Lagrangian2dPara::velocityAttenuation;
            const float eps = (std::max)(1e-6f, Lagrangian2dPara::eps);

            const glm::vec2 gravity(Lagrangian2dPara::gravityX, -Lagrangian2dPara::gravityY);

            const float h = mPs.supportRadius;
            const float h2 = mPs.supportRadius2;

            const float pi = 3.14159265358979323846f;
            const float poly6Coeff = 4.0f / (pi * (std::pow)(h, 8.0f));
            const float spikyGradCoeff = -30.0f / (pi * (std::pow)(h, 5.0f));
            const float viscLapCoeff = 40.0f / (pi * (std::pow)(h, 5.0f));

            const float particleMass = restDensity * mPs.particleVolume;

            const glm::vec2 lowerBound = mPs.lowerBound;
            const glm::vec2 upperBound = mPs.upperBound;

            const uint32_t blocksX = mPs.blockNum.x;
            const uint32_t blocksY = mPs.blockNum.y;
            const uint32_t blockCount = blocksX * blocksY;

            if (blockCount == 0 || mPs.blockExtens.size() != static_cast<size_t>(blockCount))
            {
                for (ParticleInfo2d &p : mPs.particles)
                {
                    p.density = restDensity;
                    p.pressure = 0.0f;
                    p.pressDivDens2 = 0.0f;
                    p.accleration = gravity;
                    p.velocity += p.accleration * dt;

                    const float speed = glm::length(p.velocity);
                    if (speed > maxVelocity && speed > eps)
                    {
                        p.velocity = p.velocity * (maxVelocity / speed);
                    }

                    p.position += p.velocity * dt;

                    if (p.position.x < lowerBound.x)
                    {
                        p.position.x = lowerBound.x;
                        p.velocity.x = -p.velocity.x * velocityAttenuation;
                    }
                    else if (p.position.x > upperBound.x)
                    {
                        p.position.x = upperBound.x;
                        p.velocity.x = -p.velocity.x * velocityAttenuation;
                    }

                    if (p.position.y < lowerBound.y)
                    {
                        p.position.y = lowerBound.y;
                        p.velocity.y = -p.velocity.y * velocityAttenuation;
                    }
                    else if (p.position.y > upperBound.y)
                    {
                        p.position.y = upperBound.y;
                        p.velocity.y = -p.velocity.y * velocityAttenuation;
                    }

                    p.blockId = mPs.getBlockIdByPosition(p.position);
                    if (!isValidBlockId(p.blockId, blockCount))
                    {
                        p.position = clampVec2(p.position, lowerBound, upperBound);
                        p.blockId = mPs.getBlockIdByPosition(p.position);
                        if (!isValidBlockId(p.blockId, blockCount))
                        {
                            p.blockId = 0;
                        }
                    }
                }

                mPs.updateBlockInfo();
                return;
            }

            for (ParticleInfo2d &piParticle : mPs.particles)
            {
                if (!isValidBlockId(piParticle.blockId, blockCount))
                {
                    piParticle.position = clampVec2(piParticle.position, lowerBound, upperBound);
                    piParticle.blockId = mPs.getBlockIdByPosition(piParticle.position);
                    if (!isValidBlockId(piParticle.blockId, blockCount))
                    {
                        piParticle.blockId = 0;
                    }
                }

                float densitySum = 0.0f;
                const glm::uvec2 baseCoord = blockCoordFromId(piParticle.blockId, blocksX);

                for (int dy = -1; dy <= 1; ++dy)
                {
                    const int ny = static_cast<int>(baseCoord.y) + dy;
                    if (ny < 0 || ny >= static_cast<int>(blocksY))
                    {
                        continue;
                    }

                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        const int nx = static_cast<int>(baseCoord.x) + dx;
                        if (nx < 0 || nx >= static_cast<int>(blocksX))
                        {
                            continue;
                        }

                        const uint32_t neighborBlockId = blockIdFromCoord(glm::uvec2(nx, ny), blocksX);
                        const glm::uvec2 extent = mPs.blockExtens[neighborBlockId];
                        const uint32_t startIndex = extent.x;
                        const uint32_t endIndex = extent.y;

                        if (endIndex <= startIndex)
                        {
                            continue;
                        }

                        for (uint32_t j = startIndex; j < endIndex; ++j)
                        {
                            const ParticleInfo2d &pjParticle = mPs.particles[j];
                            const glm::vec2 rij = piParticle.position - pjParticle.position;
                            const float r2 = glm::dot(rij, rij);
                            densitySum += particleMass * poly6Kernel2D(r2, h2, poly6Coeff);
                        }
                    }
                }

                piParticle.density = (std::max)(densitySum, restDensity * 0.1f);
            }

            for (ParticleInfo2d &p : mPs.particles)
            {
                const float ratio = p.density / restDensity;
                p.pressure = stiffness * ((std::pow)(ratio, exponent) - 1.0f);
                p.pressDivDens2 = p.pressure / (p.density * p.density);
            }

            for (ParticleInfo2d &piParticle : mPs.particles)
            {
                glm::vec2 pressureAcc(0.0f);
                glm::vec2 viscosityAcc(0.0f);

                const glm::uvec2 baseCoord = blockCoordFromId(piParticle.blockId, blocksX);

                for (int dy = -1; dy <= 1; ++dy)
                {
                    const int ny = static_cast<int>(baseCoord.y) + dy;
                    if (ny < 0 || ny >= static_cast<int>(blocksY))
                    {
                        continue;
                    }

                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        const int nx = static_cast<int>(baseCoord.x) + dx;
                        if (nx < 0 || nx >= static_cast<int>(blocksX))
                        {
                            continue;
                        }

                        const uint32_t neighborBlockId = blockIdFromCoord(glm::uvec2(nx, ny), blocksX);
                        const glm::uvec2 extent = mPs.blockExtens[neighborBlockId];
                        const uint32_t startIndex = extent.x;
                        const uint32_t endIndex = extent.y;

                        if (endIndex <= startIndex)
                        {
                            continue;
                        }

                        for (uint32_t j = startIndex; j < endIndex; ++j)
                        {
                            const ParticleInfo2d &pjParticle = mPs.particles[j];
                            const glm::vec2 rij = piParticle.position - pjParticle.position;
                            const float r2 = glm::dot(rij, rij);

                            if (r2 >= h2 || r2 <= 0.0f)
                            {
                                continue;
                            }

                            const float dist = (std::sqrt)(r2);

                            const glm::vec2 gradW = spikyGradKernel2D(rij, dist, h, spikyGradCoeff, eps);
                            pressureAcc -= particleMass * (piParticle.pressDivDens2 + pjParticle.pressDivDens2) * gradW;

                            const float lapW = viscosityLaplacian2D(dist, h, viscLapCoeff);
                            viscosityAcc += viscosity * particleMass * (pjParticle.velocity - piParticle.velocity) / pjParticle.density * lapW;
                        }
                    }
                }

                piParticle.accleration = pressureAcc + viscosityAcc + gravity;
            }

            for (ParticleInfo2d &p : mPs.particles)
            {
                p.velocity += p.accleration * dt;

                const float speed = glm::length(p.velocity);
                if (speed > maxVelocity && speed > eps)
                {
                    p.velocity = p.velocity * (maxVelocity / speed);
                }

                p.position += p.velocity * dt;

                if (p.position.x < lowerBound.x)
                {
                    p.position.x = lowerBound.x;
                    p.velocity.x = -p.velocity.x * velocityAttenuation;
                }
                else if (p.position.x > upperBound.x)
                {
                    p.position.x = upperBound.x;
                    p.velocity.x = -p.velocity.x * velocityAttenuation;
                }

                if (p.position.y < lowerBound.y)
                {
                    p.position.y = lowerBound.y;
                    p.velocity.y = -p.velocity.y * velocityAttenuation;
                }
                else if (p.position.y > upperBound.y)
                {
                    p.position.y = upperBound.y;
                    p.velocity.y = -p.velocity.y * velocityAttenuation;
                }

                p.blockId = mPs.getBlockIdByPosition(p.position);
                if (!isValidBlockId(p.blockId, blockCount))
                {
                    p.position = clampVec2(p.position, lowerBound, upperBound);
                    p.blockId = mPs.getBlockIdByPosition(p.position);
                    if (!isValidBlockId(p.blockId, blockCount))
                    {
                        p.blockId = 0;
                    }
                }
            }

            mPs.updateBlockInfo();
        }
    }
}
