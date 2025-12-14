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
#include "UBO.h"
#include "pointLightData.h"
#include "renderObject.h"

using namespace std;

const int SCR_WIDTH = 1600;
const int SCR_HEIGHT = 1200;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048; // 阴影贴图分辨率，越高锯齿越少
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
void initShadowMap(unsigned int& depthMapFBO, unsigned int& depthMap);

glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
    glm::vec3( 2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3( 0.0f,  0.0f, -3.0f)
};
glm::vec3 lightPos(-2.0f, 1.0f, -1.0f);

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
    Shader simpleDepthShader("shaders/simpleDepthShader.vert", "shaders/simpleDepthShader.frag");
    //天空盒
    vector<string> faces = {
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    };
    Shader reflectionShader("shaders/reflection.vert", "shaders/reflection.frag");
    reflectionShader.use();
    reflectionShader.setInt("skybox", 0); // 天空盒通常绑在 0 号纹理位
    // 这一步会自动加载纹理并配置 VAO
    Skybox skybox(faces);
    ScreenQuad screenQuad = ScreenQuad();// 设置 Shader 的 uniform
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // 6. 【核心步骤】加载模型
    Model ourModel("objects/TDA/TDA.pmx");
    Model cubeModel("objects/cube.obj");
    Model sphereModel("objects/sphere.obj");
    Model floorModel("objects/floor.obj");
    RenderObject tianyi(&ourModel);
    RenderObject cube(&cubeModel);
    RenderObject sphere(&sphereModel);
    RenderObject floor(&floorModel);

    UBO matricesUBO(2 * sizeof(glm::mat4), 0);
    UBO lightUBO(sizeof(PointLightData), 1);

    // 配置帧缓冲 (Framebuffer)
    unsigned int framebuffer;
    unsigned int textureColorBuffer;
    unsigned int depthMapFBO;
    unsigned int depthMap;
    configFrameBuffer(framebuffer, textureColorBuffer);
    initShadowMap(depthMapFBO, depthMap);

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

        // 配置UBO
        matricesUBO.SetMat4(0, projection);
        matricesUBO.SetMat4(sizeof(glm::mat4), view);
        PointLightData lightData = {
            glm::vec4(lightPos, 0.0f), // position
            glm::vec4(0.3f, 0.3f, 0.3f, 0.0f), // ambient
            glm::vec4(0.8f, 0.8f, 0.8f, 0.0f), // diffuse
            glm::vec4(1.0f, 1.0f, 1.0f, 0.0f), // specular
            1.0f, 0.09f, 0.032f, 0.0f          // constant, linear, quadratic, padding
        };
        lightUBO.SetData(0, sizeof(PointLightData), &lightData);

        // 清屏
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // ====================================================
        // 阴影帧
        // ====================================================
        // 2. 正交投影 (覆盖场景范围)
        float near_plane = 1.0f, far_plane = 5.5f;
        // 这里的数值决定了阴影覆盖的范围，太小会导致远处没阴影，太大导致精度低
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        // 3. View 矩阵 (从光的位置看向原点)
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        // 4. 合成光照空间矩阵
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        // 步骤 1: 渲染阴影贴图 (Shadow Map Pass)
        // 1. 改变视口大小 (对应阴影贴图分辨率)
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT); // 只清深度

        // 2. 使用深度 Shader
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        simpleDepthShader.setMat4("model", model);

        // 【重要】MMD 模型通常有很多单面网格。为了防止背面产生错误阴影（Peter Panning），
        // 渲染阴影贴图时，我们通常剔除正面 (只画背面)，或者不剔除。
        // 对于 Toon Shading，先试试不剔除
        glDisable(GL_CULL_FACE);
        ourModel.Draw(simpleDepthShader);

        // ==============================================
        // 第 1 遍 (Pass 1): 渲染描边
        // ==============================================
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // 清屏
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        outlineShader.use();
        outlineShader.setMat4("model", model);
        outlineShader.setFloat("outlineWidth", 0.2f); // 稍微调一点点宽度
        outlineShader.setVec3("color",glm::vec3(0.3f));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        ourModel.Draw(outlineShader);


        // ==============================================
        // 第 2 遍 (Pass 2): 正常渲染 Toon 模型
        // ==============================================
        shader.use();
        shader.setMat4("model", model);
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        shader.setFloat("material.shininess", 256.0f);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        // 3. 绑定阴影贴图
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        ourModel.Draw(shader);

        // ------------------------------------------------
        // 绘制反射箱子
        // ------------------------------------------------
        // reflectionShader.use();
        // glm::mat4 modelCube = glm::mat4(1.0f);
        // modelCube = glm::translate(modelCube, glm::vec3(-2.0f, 1.0f, 0.0f)); // 放在角色左手边
        // modelCube = glm::scale(modelCube, glm::vec3(0.5f));
        //
        // reflectionShader.setMat4("model", modelCube);
        // reflectionShader.setVec3("cameraPos", camera.Position);
        // ourModel.Draw(reflectionShader);

        // ==============================================
        // 天空盒  光源  地板
        // ==============================================
        skybox.Draw(skyboxShader, view, projection);
        cubeModel.DrawAt(lightPos,lightCubeShader);
        floorModel.DrawAt(glm::vec3(0.0f, 0.0f, 0.0f),shader);

        // ==============================================
        // 后处理
        // ==============================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        screenShader.use();
        // glBindTexture(GL_TEXTURE_2D, depthMap);
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


    // ==============================================
    // 光源控制逻辑
    // ==============================================
    float lightSpeed = 5.0f * deltaTime; // 光源移动速度

    // X轴移动 (左/右)
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos.x -= lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos.x += lightSpeed;

    // Z轴移动 (上/下)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos.z -= lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos.z += lightSpeed;

    // Y轴移动 (垂直升降) - 使用 Right Shift 和 Right Control
    if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        lightPos.y += lightSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        lightPos.y -= lightSpeed;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
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

void initShadowMap(unsigned int& depthMapFBO, unsigned int& depthMap){
    // 1. 创建 FBO
    glGenFramebuffers(1, &depthMapFBO);

    // 2. 创建深度纹理
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // 注意：这里格式是 GL_DEPTH_COMPONENT
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // 设置过滤参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 设置环绕参数：超出范围的地方不做阴影 (设为白色，深度 1.0)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // 3. 把纹理附加到 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    // 显式告诉 OpenGL：我们不需要任何颜色数据！
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}