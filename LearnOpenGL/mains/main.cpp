#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "shader.h"
#include "mesh.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.h"

using namespace std;

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true; // 用于解决第一次进入窗口时的跳变问题

// 时间控制
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- 函数声明 ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // 【新】鼠标移动回调
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // 【新】滚轮回调
void processInput(GLFWwindow *window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLFWwindow* initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL 3D Cube", NULL, NULL);
    if (window == nullptr) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return nullptr;
    }
    return window;
}

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};
int main() {
    GLFWwindow* window = initWindow();
    if (!window) return -1;

    // 注册回调函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback); // 注册鼠标移动
    glfwSetScrollCallback(window, scroll_callback);   // 注册滚轮

    // 隐藏光标并捕捉它 (FPS 模式)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --- 1. 开启深度测试 (解决穿模的关键) ---
    glEnable(GL_DEPTH_TEST);


    // --- 4. 资源加载 ---
    Shader shader("shaders/shader.vert", "shaders/shader.frag");
    Shader lightCubeShader("shaders/light_cube.vert", "shaders/light_cube.frag");

    // 6. 【核心步骤】加载模型
    // 确保 stbi_set_flip_vertically_on_load 在 Model 类内部处理，或者在这里设置
    // 我们的 Model 类构造函数里已经处理了翻转
    // 请确保路径正确！！
    Model ourModel("objects/backpack/backpack.obj");

    // 7. 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        processInput(window);

        // 清屏
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活 Shader
        shader.use();

        // 设置光源属性 (简单的点光源，跟摄像机位置稍微错开一点)
        shader.setVec3("pointLight.position", 2.0f, 2.0f, 2.0f);
        shader.setVec3("pointLight.ambient", 0.2f, 0.2f, 0.2f);
        shader.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
        shader.setFloat("pointLight.constant", 1.0f);
        shader.setFloat("pointLight.linear", 0.09f);
        shader.setFloat("pointLight.quadratic", 0.032f);

        // 设置材质反光度
        shader.setFloat("material.shininess", 32.0f);

        // 设置 View/Projection 矩阵
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);

        // 渲染模型
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 放在原点
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// 缩放 (如果模型太大或太小，调这里)
        shader.setMat4("model", model);

        // 【关键】一行代码绘制整个复杂模型
        ourModel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// --- 键盘输入处理 ---
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 只需要这一行就能动了！
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// --- 鼠标移动回调 ---
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里是相反的，因为Y坐标是从底部往上增长的

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// --- 鼠标滚轮回调 ---
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}