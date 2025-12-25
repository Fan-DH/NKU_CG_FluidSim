#include "Configure.h"

// 系统默认配置参数
int imageWidth = 600;       // 渲染分辨率宽
int imageHeight = 600;      // 渲染分辨率高

int windowWidth = 1080;     // 默认窗口宽度
int windowHeight = 960;     // 默认窗口高度

float fontSize = 16.0f;     // 默认字体大小

bool simulating = false;    // 当前是否处于仿真状态

// 2D 欧拉流体模拟参数
namespace Eulerian2dPara
{
    // MAC 网格相关
    int theDim2d[2] = {100, 100};   // 网格维度
    float theCellSize2d = 0.5;      // 网格单元尺寸
    
    // 烟雾源及其参数
    std::vector<SourceSmoke> source = {
        {   // 源位置                         // 初始速度           // 密度  // 温度
            glm::ivec2(theDim2d[0] / 3, 0), glm::vec2(0.0f, 1.0f), 1.0f, 1.0f
        }
    };

    bool addSolid = true;           // 是否添加固体边界

    // 可视化相关
    float contrast = 1;             // 烟雾对比度
    int drawModel = 0;              // 绘制模式
    int gridNum = theDim2d[0];      // 用于显示的网格数量

    // 物理参数
    float dt = 0.01;                // 时间步长
    float airDensity = 1.3;         // 空气密度
    float ambientTemp = 0.0;        // 环境温度
    float boussinesqAlpha = 500.0;  // Boussinesq 公式中的 alpha 系数
    float boussinesqBeta = 2500.0;  // Boussinesq 公式中的 beta 系数
}

// 3D 欧拉流体模拟参数
namespace Eulerian3dPara
{
    // MAC 网格相关
    int theDim3d[3] = {12, 36, 36}; // 网格维度（保证 x <= y = z）
    float theCellSize3d = 0.5;      // 网格单元尺寸
    std::vector<SourceSmoke> source = {
        {glm::ivec3(theDim3d[0] / 2, theDim3d[1] / 2, 0), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f}
    };
    bool addSolid = true;           // 是否添加固体边界

    // 可视化相关
    float contrast = 1;             // 烟雾对比度
    bool oneSheet = true;           // 是否只显示一张切片
    float distanceX = 0.51;         // X 方向切片位置
    float distanceY = 0.51;         // Y 方向切片位置
    float distanceZ = 0.985;        // Z 方向切片位置
    bool xySheetsON = true;         // 是否显示 XY 平面切片
    bool yzSheetsON = true;         // 是否显示 YZ 平面切片
    bool xzSheetsON = true;         // 是否显示 XZ 平面切片
    int drawModel = 0;              // 绘制模式
    int gridNumX = (int)((float)theDim3d[0] / theDim3d[2] * 100);  // X 方向网格数
    int gridNumY = (int)((float)theDim3d[1] / theDim3d[2] * 100);  // Y 方向网格数
    int gridNumZ = 100;             // Z 方向网格数
    int xySheetsNum = 3;            // XY 平面切片数量
    int yzSheetsNum = 3;            // YZ 平面切片数量
    int xzSheetsNum = 3;            // XZ 平面切片数量

    // 物理参数
    float dt = 0.01;                // 时间步长
    float airDensity = 1.3;         // 空气密度
    float ambientTemp = 0.0;        // 环境温度
    float boussinesqAlpha = 500.0;  // Boussinesq 公式中的 alpha 系数
    float boussinesqBeta = 2500.0;  // Boussinesq 公式中的 beta 系数
}

// 2D 拉格朗日流体（粒子法）模拟参数
namespace Lagrangian2dPara
{
    // 尺度系数
    float scale = 2;

    // 流体初始区域设置
    std::vector<FluidBlock> fluidBlocks = {
        {   // 左下角坐标             // 右上角坐标           // 初始速度              // 粒子间距
            glm::vec2(-0.4f, -0.4f), glm::vec2(0.4f, 0.4f), glm::vec2(0.0f, 0.0f), 0.02f
        }
    };

    // 时间积分相关参数
    float dt = 0.0016;              // 时间步长
    int substep = 1;                // 子步数
    float maxVelocity = 10;         // 速度上限
    float velocityAttenuation = 0.7; // 碰撞后的速度衰减系数
    float eps = 1e-5;               // 边界处理使用的小距离，防止粒子穿透边界

    // SPH 粒子系统参数
    float supportRadius = 0.04;      // 影响半径
    float particleRadius = 0.01;     // 粒子半径
    float particleDiameter = particleRadius * 2.0;  // 粒子直径
    float gravityX = 0.0f;          // x 方向重力加速度
    float gravityY = 9.8f;          // y 方向重力加速度
    float density = 1000.0f;        // 目标流体密度
    float stiffness = 70.0f;        // 状态方程刚度
    float exponent = 7.0f;          // 状态方程指数
    float viscosity = 0.03f;        // 粘度
    bool enableFountain2d = false;
}

// 3D 拉格朗日流体（粒子法）模拟参数
namespace Lagrangian3dPara
{
    // 尺度系数
    float scale = 1.2;
    
    // 流体初始区域设置
    std::vector<FluidBlock> fluidBlocks = {
        {
            glm::vec3(0.05, 0.05, 0.3), glm::vec3(0.45, 0.45, 0.7), glm::vec3(0.0, 0.0, -1.0), 0.02f
        },
        {
            glm::vec3(0.45, 0.45, 0.3), glm::vec3(0.85, 0.85, 0.7), glm::vec3(0.0, 0.0, -1.0), 0.02f
        }   
    };
    
    // 时间积分相关参数
    float dt = 0.002;               // 时间步长
    int substep = 1;                // 子步数
    float maxVelocity = 10;         // 速度上限
    float velocityAttenuation = 0.7; // 碰撞后的速度衰减系数
    float eps = 1e-5;               // 边界处理使用的小距离

    // SPH 粒子系统参数
    float supportRadius = 0.04;      // 影响半径
    float particleRadius = 0.01;     // 粒子半径
    float particleDiameter = particleRadius * 2.0;  // 粒子直径

    float gravityX = 0.0f;          // x 方向重力加速度
    float gravityY = 0.0f;          // y 方向重力加速度
    float gravityZ = 9.8f;          // z 方向重力加速度

    float density = 1000.0f;        // 目标流体密度
    float stiffness = 20.0f;        // 状态方程刚度
    float exponent = 7.0f;          // 状态方程指数
    float viscosity = 8e-5f;        // 粘度
}

// 存储系统中可选的仿真组件
std::vector<Glb::Component *> methodComponents;

// 资源路径
std::string shaderPath = "../../../../code/resources/shaders";
std::string picturePath = "../../../../code/resources/pictures";