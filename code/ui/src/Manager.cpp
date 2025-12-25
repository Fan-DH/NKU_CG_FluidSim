/**
 * Manager.cpp: UI 管理器实现文件
 * 负责 UI 系统的管理与视图显示
 */

#include "Manager.h"

namespace FluidSimulation
{
    /**
     * 初始化管理器
     * 创建视图并注册可用的仿真方法组件
     * @param window GLFW 窗口
     */
    void Manager::init(GLFWwindow* window) {
        // 保存窗口指针
        this->window = window;
        dockLayoutBuilt = false;

        // 创建各个视图
        inspectorView = new InspectorView(window);
        projectView =  new ProjectView(window);
        sceneView = new SceneView(window);

        // 创建并注册可选的仿真方法组件
        int id = 0;
        methodComponents.push_back(new Lagrangian2d::Lagrangian2dComponent("Lagrangian 2d", id++));
        methodComponents.push_back(new Eulerian2d::Eulerian2dComponent("Eulerian 2d", id++));
        methodComponents.push_back(new Lagrangian3d::Lagrangian3dComponent("Lagrangian 3d", id++));
        methodComponents.push_back(new Eulerian3d::Eulerian3dComponent("Eulerian 3d", id++));
        // TODO(optional): 添加更多仿真方法
    }

    /**
     * 显示所有视图
     * 创建 DockSpace，并显示场景视图、检查器视图和项目视图
     */
	void Manager::displayViews() {
        // DockSpace 标志
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);

        if (!dockLayoutBuilt && viewport != nullptr) {
            const ImGuiID dockspace_id = viewport->ID;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

            ImGuiID dock_id_main = dockspace_id;
            const ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.25f, nullptr, &dock_id_main);
            const ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.28f, nullptr, &dock_id_main);

            ImGui::DockBuilderDockWindow("Scene", dock_id_main);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Project", dock_id_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
            dockLayoutBuilt = true;
        }

        // 统一的窗口样式设置
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

        // 显示各个视图
        sceneView->display();
        inspectorView->display();
        projectView->display();

        ImGui::PopStyleColor(5);
	}

    /**
     * 显示工具栏
     * TODO: 实现工具栏内容
     */
    void Manager::displayToolBar() {
        // TODO: 实现工具栏
    }
}
