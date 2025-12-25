#pragma once
#ifndef __LAGRANGIAN_2D_SOLVER_H__
#define __LAGRANGIAN_2D_SOLVER_H__

#include "ParticleSystem2d.h"
#include "Configure.h"

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // 拉格朗日法流体求解器类
        // 负责求解基于粒子的流体动力学方程
        class Solver
        {
        public:
            Solver(ParticleSystem2d &ps);

            void solve();                         // 求解流体方程

        private:
            ParticleSystem2d &mPs;               // 粒子系统引用
        };
    }
}

#endif
