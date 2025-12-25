#pragma once
#ifndef __LAGRANGIAN_3D_SOLVER_H__
#define __LAGRANGIAN_3D_SOLVER_H__

#include "ParticleSystem3d.h"
#include "Global.h"
#include "Configure.h"
#include <iostream>

namespace FluidSimulation
{
    namespace Lagrangian3d
    {
        // 拉格朗日法流体求解器类
        // 负责求解基于粒子的三维流体动力学方程
        class Solver
        {
        public:
            Solver(ParticleSystem3d &ps);

            void solve();                         // 求解流体方程

        private:
            ParticleSystem3d &mPs;               // 粒子系统引用
        };
    }
}

#endif
