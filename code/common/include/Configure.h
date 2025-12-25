/**
 * Configure.h: 系统配置文件
 * 定义系统使用的各种参数和全局变量
 */

#pragma once
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include <iostream>
#include <string>
#include "Component.h"
#include <vector>
#include "glm/glm.hpp"

/**
 * 线性插值宏
 * @param a 起始值
 * @param b 结束值
 * @param t 插值参数 [0,1]
 */
#define LERP(a, b, t) (1 - t) * a + t *b

#ifndef __MINMAX_DEFINED
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// 渲染分辨率
extern int imageWidth;      // 渲染图像宽度
extern int imageHeight;     // 渲染图像高度

// 窗口尺寸
extern int windowWidth;     // 窗口宽度
extern int windowHeight;    // 窗口高度

// UI 设置
extern float fontSize;      // 字体大小

// 仿真状态
extern bool simulating;     // 是否正在进行仿真

/**
 * 2D 欧拉流体模拟参数命名空间
 * 存放 2D 欧拉流体模拟相关的配置参数
 */
namespace Eulerian2dPara
{
    /**
     * 烟雾源数据结构
     * 描述烟雾源的初始位置、速度、密度和温度
     */
    struct SourceSmoke {
        glm::ivec2 position = glm::ivec2(0);
        glm::vec2 velocity = glm::vec2(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim2d[];
    extern std::vector<SourceSmoke> source;
    extern float theCellSize2d;
    extern bool addSolid;

    extern float dt;

    extern float contrast;
    extern int drawModel;
    extern int gridNum;


    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;
}

/**
 * 3D 欧拉流体模拟参数命名空间
 * 存放 3D 欧拉流体模拟相关的配置参数
 */
namespace Eulerian3dPara
{
    /**
     * 烟雾源数据结构
     * 描述烟雾源的初始位置、速度、密度和温度
     */
    struct SourceSmoke {
        glm::ivec3 position = glm::ivec3(0);
        glm::vec3 velocity = glm::vec3(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim3d[];
    extern float theCellSize3d;
    extern std::vector<SourceSmoke> source;
    extern bool addSolid;

    extern float contrast;
    extern bool oneSheet;
    extern float distanceX;
    extern float distanceY;
    extern float distanceZ;
    extern bool xySheetsON;
    extern bool yzSheetsON;
    extern bool xzSheetsON;
    extern int xySheetsNum;
    extern int yzSheetsNum;
    extern int xzSheetsNum;
    extern int drawModel;
    extern int gridNumX;
    extern int gridNumY;
    extern int gridNumZ;

    extern float dt;

    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;

}

/**
 * 2D 拉格朗日流体（粒子法）参数命名空间
 * 存放 2D 粒子流体模拟相关的配置参数
 */
namespace Lagrangian2dPara
{
    /**
     * 流体块数据结构
     * 描述流体初始包围盒、初始速度以及粒子间距
     */
    struct FluidBlock {
        glm::vec2 lowerCorner = glm::vec2(0.0f, 0.0f);
        glm::vec2 upperCorner = glm::vec2(0.0f, 0.0f);
        glm::vec2 initVel = glm::vec2(0.0f, 0.0f);
        float particleSpace = 0.02f;
    };

    extern float scale;
    extern std::vector<FluidBlock> fluidBlocks;

    extern float dt;
    extern int substep;
    extern float maxVelocity;
    extern float velocityAttenuation;
    extern float eps;

    extern float supportRadius;
    extern float particleRadius;
    extern float particleDiameter;
    extern float gravityX;
    extern float gravityY;
    extern float density;
    extern float stiffness;
    extern float exponent;
    extern float viscosity;
    extern bool enableFountain2d;
}

/**
 * 3D 拉格朗日流体（粒子法）参数命名空间
 * 存放 3D 粒子流体模拟相关的配置参数
 */
namespace Lagrangian3dPara
{
    /**
     * 流体块数据结构
     * 描述流体初始包围盒、初始速度以及粒子间距
     */
    struct FluidBlock {
        glm::vec3 lowerCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upperCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 initVel = glm::vec3(0.0f, 0.0f, 0.0f);
        float particleSpace = 0.02f;
    };

    extern float scale;
    extern std::vector<FluidBlock> fluidBlocks;


    extern float dt;
    extern int substep;
    extern float maxVelocity;
    extern float velocityAttenuation;
    extern float eps;

    extern float supportRadius;
    extern float particleRadius;
    extern float particleDiameter;

    extern float gravityX;
    extern float gravityY;
    extern float gravityZ;

    extern float density;
    extern float stiffness;
    extern float exponent;
    extern float viscosity;
}

// 资源路径
extern std::string shaderPath;       // 着色器文件路径
extern std::string picturePath;      // 纹理图片文件路径

// 仿真方法组件列表
extern std::vector<Glb::Component *> methodComponents;  // 所有仿真方法组件列表

#endif // !__CONFIGURE_H__
