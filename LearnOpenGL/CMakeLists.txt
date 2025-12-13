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
file(GLOB SOURCE_FILES "mains/main*.cpp")
message(STATUS "扫描到的文件列表: ${SOURCE_FILES}")
foreach(ONE_SOURCE_FILE ${SOURCE_FILES})
    # 获取文件名（去掉路径和扩展名），例如 main_triangle.cpp -> main_triangle
    get_filename_component(EXE_NAME ${ONE_SOURCE_FILE} NAME_WE)

    # 创建可执行文件 (注意：不用再写 glad.c 了，因为下面会链接 glad 库)
    add_executable(${EXE_NAME} ${ONE_SOURCE_FILE})

    # 链接库：链接 GLFW, GLAD库, 和系统OpenGL库
    target_link_libraries(${EXE_NAME} PRIVATE glfw glad ${OS_LIBS})

    message(STATUS "已添加可执行文件: ${EXE_NAME}")
endforeach()