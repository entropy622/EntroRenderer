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
#include "screenQuad.h"
#include "skybox.h"

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
GLFWwindow* initWindow();
void configFrameBuffer(unsigned int &framebuffer, unsigned int &textureColorBuffer);
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

    // --- 4. 资源加载 ---
    Shader shader("shaders/shader.vert", "shaders/toon_shader.frag");
    Shader outlineShader("shaders/outline.vert", "shaders/outline.frag");
    Shader screenShader("shaders/screen.vert", "shaders/screen.frag");
    Shader lightCubeShader("shaders/light_cube.vert", "shaders/light_cube.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");
    //天空盒
    vector<string> faces = {
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    };
    // 这一步会自动加载纹理并配置 VAO
    Skybox skybox(faces);
    ScreenQuad screenQuad = ScreenQuad();// 设置 Shader 的 uniform
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // 6. 【核心步骤】加载模型
    Model ourModel("objects/TDA/TDA.pmx");

    // 配置帧缓冲 (Framebuffer)
    unsigned int framebuffer;
    unsigned int textureColorBuffer;
    configFrameBuffer(framebuffer, textureColorBuffer);

    // 开启混合
    glEnable(GL_BLEND);
    // 设置混合方程式：SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor
    // 翻译：新颜色的浓度取决于它的透明度，剩下的浓度留给背景色
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // 7. 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);

        // 时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入
        processInput(window);

        // 设置 View/Projection 矩阵
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // transform
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 放在原点
        model = glm::scale(model, glm::vec3(0.1));	// 缩放 (如果模型太大或太小，调这里)

        // ==============================================
        // 第 1 遍 (Pass 1): 渲染描边
        // ==============================================
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // 清屏
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        outlineShader.use();

        outlineShader.setMat4("view", view);
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("model", model);
        outlineShader.setFloat("outlineWidth", 0.2f); // 稍微调一点点宽度
        outlineShader.setVec3("color",0.0f, 0.0f, 0.0f);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        ourModel.Draw(outlineShader);


        // ==============================================
        // 第 2 遍 (Pass 2): 正常渲染 Toon 模型
        // ==============================================
        shader.use();
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        shader.setVec3("pointLight.position", 5.0f, 5.0f, 5.0f);
        // --- 调整光照颜色 ---
        // 环境光不要太强，漫反射要亮
        shader.setVec3("pointLight.ambient", 0.3f, 0.3f, 0.3f);
        shader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f); // 主光要亮
        shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f); // 高光纯白
        // MMD 的高光是很锐利的，把这个值设置得很高（比如 128, 256 甚至更高）
        shader.setFloat("material.shininess", 256.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);
        shader.setVec3("viewPos", camera.Position);
        ourModel.Draw(shader);

        // ==============================================
        // 天空盒
        // ==============================================
        skybox.Draw(skyboxShader, view, projection);

        // ==============================================
        // 后处理
        // ==============================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        screenShader.use();
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        screenQuad.Draw();

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

void configFrameBuffer(unsigned int &framebuffer, unsigned int &textureColorBuffer) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 1. 生成纹理附件 (Color Attachment Texture)
    // 我们把画面渲染到这张纹理上，而不是直接渲染到屏幕

    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    // 2. 生成渲染缓冲对象 (Renderbuffer Object) 用于深度和模板
    // 我们需要深度测试来正确渲染 3D 模型，所以必须加这个
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // 3. 检查完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

    // 解绑，切回默认帧缓冲 (避免意外渲染到 FBO)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};