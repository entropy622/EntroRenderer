#define STB_IMAGE_IMPLEMENTATION
#include "texture.h" // 放在最前面引入 stb_image

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
#include "cubeData.h"

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

glm::vec3 lightPos(1.2f, 1.0f, 2.0f); // 把灯放在右上方
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

    // --- 2. 准备数据 (使用结构体初始化，非常直观) ---
    // 这里的 {} 会自动对应 Vertex 结构体里的 {Position, TexCoords}

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
      };

    // --- 3. 创建网格对象 (自动配置 VAO/VBO) ---
    Mesh cubeMesh(CubeData::GetCubeVertices());

    // --- 4. 资源加载 ---
    Shader shader("shaders/shader.vert", "shaders/shader.frag");
    Shader lightCubeShader("shaders/light_cube.vert", "shaders/light_cube.frag");
    Texture texture1("textures/container2.png");
    // Texture texture2("textures/awesomeface.jpg");
    Texture texture2("textures/container2_specular.png");
    Texture emissionMap("textures/matrix.jpg");

    shader.use();
    shader.setInt("material.diffuse", 0);
    shader.setInt("material.specular", 1);
    shader.setInt("material.emission", 2);

    // --- 5. 渲染循环 ---
    while (!glfwWindowShouldClose(window)) {
        // 1. 计算时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 2. 处理键盘输入
        processInput(window);

        // 清空颜色 和 深度缓冲
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        texture1.bind(0);
        texture2.bind(1);
        emissionMap.bind(2);

        // --- 矩阵变换 ---
        // 1. Model:
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        // 2. View:
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        // 3. Projection:
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        // --- 设置光源属性 (通常是白光) ---
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient",  0.1f, 0.1f, 0.1f); // 稍微暗一点的环境光
        shader.setVec3("light.diffuse",  1.0f, 1.0f, 1.0f); // 正常的漫反射光
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f); // 最亮的镜面光
        shader.setFloat("light.constant",  1.0f);
        shader.setFloat("light.linear",    0.09f);
        shader.setFloat("light.quadratic", 0.032f);
        
        shader.setVec3("material.specular", glm::vec3(0.6f)); // 翡翠的高光也是绿的
        shader.setFloat("material.shininess", 0.6f * 128.0f);

        for(unsigned int i = 0; i < 10; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
            shader.setMat4("model", model);

            cubeMesh.Draw();
        }

        // --- 绘制 ---
        cubeMesh.Draw();

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos); // 把它移到光源位置
        model = glm::scale(model, glm::vec3(0.2f)); // 把它缩小一点，像个灯泡
        lightCubeShader.setMat4("model", model);
        lightCubeShader.setMat4("model", model);
        cubeMesh.Draw();


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源清理
    cubeMesh.Delete();
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