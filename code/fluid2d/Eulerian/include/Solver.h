/**
 * Solver.h: 2D欧拉流体求解器头文件
 * 定义流体仿真求解器类
 */

#pragma once
#ifndef __EULERIAN_2D_SOLVER_H__
#define __EULERIAN_2D_SOLVER_H__

#include "Eulerian/include/MACGrid2d.h"
#include "Global.h"

namespace FluidSimulation{
	namespace Eulerian2d {
		/**
		 * 求解器类
		 * 实现基于MAC网格的2D欧拉流体仿真算法
		 */
		class Solver {
		public:
			/**
			 * 构造函数
			 * @param grid MAC网格引用
			 */
			Solver(MACGrid2d& grid);

			/**
			 * 执行一步仿真计算
			 * 包含速度更新、压力求解等步骤
			 */
			void solve();

		protected:
			MACGrid2d& mGrid;  // MAC网格引用
		};
	}
}

#endif // !__EULER_SOLVER_H__
