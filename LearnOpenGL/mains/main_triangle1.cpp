#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
using namespace std;
// 窗口大小变化时的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void createShader(unsigned int& shaderProgram, const char *vertexShaderSource, const char *fragmentShaderSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void bindData(unsigned int& VAO, unsigned int& VBO, const vector<float>& vertices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

int main() {
    // 1. 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac用户需要这一行
#endif

    // 2. 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL in CLion", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. 初始化 GLAD (加载OpenGL函数指针)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    float firstTriangle[] = {
        -0.9f, -0.5f, 0.0f,  // left
        -0.0f, -0.5f, 0.0f,  // right
        -0.45f, 0.5f, 0.0f,  // top
    };
    float secondTriangle[] = {
        0.0f, -0.5f, 0.0f,  // left
        0.9f, -0.5f, 0.0f,  // right
        0.45f, 0.5f, 0.0f   // top
    };
    const char *vertexShaderSource = "#version 420 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


    const char *fragmentShaderSource = "#version 420 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";


    unsigned int shaderProgram = glCreateProgram();
    unsigned int shaderProgram1 = glCreateProgram();
    createShader(shaderProgram, vertexShaderSource, fragmentShaderSource);

    unsigned int VAOs[2], VBOs[2];
    bindData(VAOs[0], VBOs[0], vector<float>(firstTriangle, firstTriangle + sizeof(firstTriangle) / sizeof(float)));
    bindData(VAOs[1], VBOs[1], vector<float>(secondTriangle, secondTriangle + sizeof(secondTriangle) / sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 设置为线框模式
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 4. 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // --- 渲染指令放这里 ---

        // 设置清空屏幕的颜色 (这里是深青色)
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // --- 结束渲染 ---

        glfwSwapBuffers(window); // 交换缓冲
        glfwPollEvents();        // 检查事件(键盘输入等)
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}