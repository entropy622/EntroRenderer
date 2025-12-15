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

# ... 下载 Assimp ...
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.2.5
)

# 【核心修改】开启 DLL 模式 (Shared Libs)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

# 保持其他优化
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
# 配置 Dear ImGui
# ==========================================

# 1. 设置 ImGui 的路径变量 (根据你的实际目录修改这里!)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)

# 2. 收集 ImGui 的源文件 (核心 + 后端)
# 注意：ImGui 不是只有头文件，必须编译这些 .cpp
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        # GLFW 后端
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        # OpenGL3 后端
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# 3. 创建 ImGui 静态库
# 这样 CMake 会把它编译成 imgui.lib (或 .a)，而不是混在你的 main.exe 里
add_library(imgui STATIC ${IMGUI_SOURCES})

# 4. 设置头文件包含路径
# PUBLIC 意味着：链接了 imgui 的目标（你的 main.exe）也能自动找到这些头文件
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

# 5. 链接依赖
# ImGui 的后端文件里用到了 GLFW 和 OpenGL 函数，所以它必须链接它们
# 假设你的 GLFW 库叫 'glfw' 或 'glfw3'
target_link_libraries(imgui PRIVATE glfw)

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
    target_link_libraries(${EXE_NAME} PRIVATE MyCore imgui)
    if(WIN32)
        add_custom_command(TARGET ${EXE_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:assimp>  # 找到 assimp 编译出来的 dll
                $<TARGET_FILE_DIR:${EXE_NAME}> # 复制到 main.exe 所在的目录
        )
    endif()

    message(STATUS "已添加可执行文件: ${EXE_NAME}")
endforeach()