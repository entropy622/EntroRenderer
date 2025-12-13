cmake_minimum_required(VERSION 3.24)
project(LearnOpenGL_CLion)

set(CMAKE_CXX_STANDARD 17)

# ==========================================
# 1. 自动下载并配置 GLFW
# ==========================================
include(FetchContent)
message(STATUS "正在配置 GLFW...")
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ==========================================
# 2. 自动下载 GLM (纯头文件)
# ==========================================
message(STATUS "正在配置 GLM...")
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)
FetchContent_GetProperties(glm)
if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
endif()

# 下载 Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.2.5  # 使用一个稳定的版本
)
# 注意：Assimp 编译很慢，我们可以关闭一些不需要的格式来加速
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# ==========================================
# 3. 基础路径配置
# ==========================================
include_directories(${CMAKE_SOURCE_DIR}/include) # 你的 glad 头文件位置
include_directories(${glm_SOURCE_DIR})           # GLM 头文件位置

# ==========================================
# 4. 优化：把 GLAD 编译成静态库
# ==========================================
# 这样就不用在每个 main 里都把 glad.c 编译一遍了，只编译一次，大家一起用
add_library(glad STATIC glad.c)
# Linux 下 glad 需要链接 dl 库
if (UNIX AND NOT APPLE)
    target_link_libraries(glad PRIVATE dl)
endif()

# ==========================================
# 5. 定义平台相关的链接库 (OS_LIBS)
# ==========================================
if (WIN32)
    set(OS_LIBS opengl32)
elseif (APPLE)
    set(OS_LIBS "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
else ()
    set(OS_LIBS GL)
endif ()

# ==========================================
# 6. 【核心修改】自动扫描 main_*.cpp 并生成可执行文件
# ==========================================
# 查找目录下所有以 main_ 开头的 .cpp 文件

# 1. 扫描 src 下的所有实现文件 (Mesh.cpp, Model.cpp)
file(GLOB IMPL_SOURCES "src/*.cpp")

# 2. 把这些实现文件编译成一个静态库 (就像我们对 GLAD 做的那样)
# 这样 Mesh.cpp 和 Model.cpp 只会被编译一次，大大加快后续编译速度！
add_library(MyCore STATIC ${IMPL_SOURCES})

# MyCore 库依赖 Assimp, GLAD, GLFW 等
# 这里的 PUBLIC 意味着谁链接了 MyCore，谁也能自动找到 Assimp 的头文件
target_link_libraries(MyCore PUBLIC glad glfw assimp::assimp ${OS_LIBS})


# 3. 扫描 mains 下的所有入口文件
file(GLOB MAIN_SOURCES "mains/main*.cpp")

message(STATUS "扫描到的主程序: ${MAIN_SOURCES}")

foreach(ONE_SOURCE_FILE ${MAIN_SOURCES})
    # 获取文件名，例如 main_triangle.cpp -> main_triangle
    get_filename_component(EXE_NAME ${ONE_SOURCE_FILE} NAME_WE)

    # 创建可执行文件 (只包含 main_xxx.cpp)
    add_executable(${EXE_NAME} ${ONE_SOURCE_FILE})

    # 链接我们刚才创建的 MyCore 库
    # 这样 main_xxx 就能用 MyCore 里的 Mesh 和 Model 类了
    target_link_libraries(${EXE_NAME} PRIVATE MyCore)

    message(STATUS "已添加可执行文件: ${EXE_NAME}")
endforeach()