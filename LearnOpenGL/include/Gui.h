#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h> // 需要 GLFWwindow 定义

class Gui {
public:
    // 构造函数：初始化 ImGui
    Gui(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImFontConfig config;
        config.SizePixels = 24.0f;
        io.Fonts->AddFontDefault(&config);
        // 同时缩放控件的样式 (按钮大小、间距等)
        ImGui::GetStyle().ScaleAllSizes(2.0f); // 2.0f 代表放大两倍

        // 设置后端
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 420");
    }

    // 析构函数：程序退出时自动清理，再也不会忘写 Shutdown 了！
    ~Gui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    // 每一帧开始时调用
    void BeginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // 每一帧结束时调用 (绘制 UI)
    void EndFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // 具体的面板绘制逻辑
    // 传入引用，这样我们就能直接修改 main.cpp 里的变量
    void DrawPanel(PointLightData& lightData, float& exposure) {
        ImGui::Begin("Scene Controls");

        ImGui::Text("Performance: %.1f FPS", ImGui::GetIO().Framerate);

        if (ImGui::CollapsingHeader("Post Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f);
        }

        if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // 1. 位置控制 (操作 lightData.position.x, y, z)
            ImGui::Text("Transform");
            ImGui::DragFloat3("Position", &lightData.position.x, 0.1f);

            // 2. 颜色控制
            // ImGui::ColorEdit3 需要 float* 指针，glm::vec4 的 .x 地址正好对应 r,g,b 连续内存
            ImGui::Separator();
            ImGui::Text("Colors");
            ImGui::ColorEdit3("Ambient", &lightData.ambient.x);
            ImGui::ColorEdit3("Diffuse", &lightData.diffuse.x);
            ImGui::ColorEdit3("Specular", &lightData.specular.x);

            // 3. 衰减控制 (Attenuation)
            // linear 和 quadratic 通常是很小的小数，所以 step 要设得很小
            ImGui::Separator();
            ImGui::Text("Attenuation");
            ImGui::DragFloat("Constant", &lightData.constant, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Linear", &lightData.linear, 0.001f, 0.0f, 1.0f, "%.4f");
            ImGui::DragFloat("Quadratic", &lightData.quadratic, 0.0001f, 0.0f, 1.0f, "%.5f");
        }
        ImGui::End();
    }
};

#endif //GUI_H