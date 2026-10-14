cmake_minimum_required(VERSION 3.20)

project(VisionFrame VERSION 1.0 LANGUAGES CXX)

# =============================================================================
# 构建输出目录（与原 MSBuild 布局保持一致）
#   - 可执行文件 -> ${CMAKE_SOURCE_DIR}/App            (VisionFrame.exe)
#   - 动态库     -> ${CMAKE_SOURCE_DIR}/App/Bin        (*.dll 及其运行期依赖)
#   - 导入库/静态库 -> ${CMAKE_SOURCE_DIR}/Include/lib (*.lib)
#   - 公共头文件 -> ${CMAKE_SOURCE_DIR}/Include/include
# =============================================================================
set(VF_APP_DIR       "${CMAKE_SOURCE_DIR}/App")
set(VF_BIN_DIR       "${VF_APP_DIR}/Bin")
set(VF_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/Include/include")
set(VF_LIB_DIR       "${CMAKE_SOURCE_DIR}/Include/lib")

# Windows 上 DLL 属于 runtime 产物，输出到 App/Bin；exe 通过 target 属性覆盖到 App/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         "${VF_BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY         "${VF_BIN_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY         "${VF_LIB_DIR}")
# 多配置生成器（VS）下按配置分目录会破坏 App/Bin 的 DLL 搜索路径，统一扁平输出
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(CONCAT CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} "${VF_BIN_DIR}")
    string(CONCAT CMAKE_LIBRARY_OUTPUT_DIRECTORY_${cfg} "${VF_BIN_DIR}")
    string(CONCAT CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${cfg} "${VF_LIB_DIR}")
endforeach()

# =============================================================================
# 全局编译选项
# =============================================================================
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

if(MSVC)
    add_compile_options(/utf-8)
    # 关闭 SDL 检查与默认的 _CRT_SECURE_NO_WARNINGS 警告噪声，保持与原工程一致
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _WINDLL)
    # Qt5.14.2 使用了 stdext::make_checked_array_iterator，较新 MSVC STL 默认禁用，
    # 需显式开启 _HAS_DEPRECATED_ITERATOR 以恢复 stdext 命名空间
    add_compile_definitions(_HAS_DEPRECATED_ITERATOR=1 _SILENCE_STDEXT_DEPRECATION_WARNING)
    # 多处理器编译
    add_compile_options(/MP)

    # Release 配置同时生成调试信息（PDB），支持断点调试
    # 默认 Release 只有 /O2 /Ob2 /DNDEBUG，不含 /Zi；此处补充 /Zi 与链接器 /DEBUG
    add_compile_options($<$<CONFIG:Release>:/Zi>)
    add_compile_options($<$<CONFIG:Release>:/FS>)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# 默认以 x64 为目标架构（与原 MSBuild Release|x64 一致）；跨平台/边缘部署放行 ARM64
if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)|(amd64)|(arm64)|(aarch64)|(ARM64)")
    message(FATAL_ERROR "VisionFrame 仅支持 x86_64 / ARM64 架构。当前: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Linux：可执行/共享库运行期相互定位（exe 在 App/，共享库在 App/Bin/，插件在 App/Bin/plugins/）
if(UNIX)
    set(CMAKE_BUILD_RPATH   "$ORIGIN;$ORIGIN/Bin;$ORIGIN/..")
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/Bin;$ORIGIN/..")
endif()

# =============================================================================
# 查找 Qt5（默认指向 5.14.2 msvc2017_64 安装位置，可由 -DQT_DIR 覆盖）
# =============================================================================
if(NOT DEFINED QT_DIR)
    set(QT_DIR_DEFAULT "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")
    if(EXISTS "${QT_DIR_DEFAULT}")
        set(QT_DIR "${QT_DIR_DEFAULT}" CACHE PATH "Qt5 CMake config 目录")
    else()
        set(QT_DIR "" CACHE PATH "Qt5 CMake config 目录（例如 D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5）")
    endif()
endif()
if(QT_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${QT_DIR}")
endif()
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# 自动 MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================
# OpenCV（双轨）：优先系统安装（Linux/边缘侧，apt/源码均可），回退仓库自带 Windows 预编译包。
# 统一暴露为 vf::opencv 目标与 OpenCV_LIBS / OpenCV_INCLUDE_DIRS 变量，下游写法不变。
# =============================================================================
add_library(vf::opencv INTERFACE IMPORTED)

find_package(OpenCV QUIET)  # 设置 OpenCV_FOUND / OpenCV_INCLUDE_DIRS / OpenCV_LIBS
if(OpenCV_FOUND)
    set_target_properties(vf::opencv PROPERTIES INTERFACE_LINK_LIBRARIES "${OpenCV_LIBS}")
    message(STATUS "OpenCV: 使用系统安装 ${OpenCV_VERSION} (${OpenCV_INCLUDE_DIRS})")
else()
    message(STATUS "OpenCV: 未找到系统安装，使用仓库自带 Windows 预编译包 4.1.2.0")

    set(OPENCV_ROOT     "${CMAKE_SOURCE_DIR}/Include/opencv4120"          CACHE PATH "OpenCV 安装根")
    set(OPENCV_INCLUDE  "${OPENCV_ROOT}/include"                         CACHE PATH "OpenCV 头文件目录")
    set(OPENCV_LIB_DIR  "${OPENCV_ROOT}/x64/lib"                          CACHE PATH "OpenVC 导入库目录")
    set(OPENCV_BIN_DIR  "${OPENCV_ROOT}/x64/bin"                          CACHE PATH "OpenCV DLL 目录")

    # 对外暴露为 IMPORTED 目标，下游 target 直接 link 即可拿到 include 路径与导入库。
    # 通过 IMPORTED_CONFIGURATIONS 让 CMake 按 Debug/Release 选择对应的 .lib/.dll。
    add_library(opencv_world SHARED IMPORTED)
    set_target_properties(opencv_world PROPERTIES
        IMPORTED_CONFIGURATIONS   "Debug;Release"
        IMPORTED_LOCATION_DEBUG    "${OPENCV_BIN_DIR}/opencv_world4120d.dll"
        IMPORTED_LOCATION_RELEASE  "${OPENCV_BIN_DIR}/opencv_world4120.dll"
        IMPORTED_IMPLIB_DEBUG      "${OPENCV_LIB_DIR}/opencv_world4120d.lib"
        IMPORTED_IMPLIB_RELEASE    "${OPENCV_LIB_DIR}/opencv_world4120.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENCV_INCLUDE}"
    )
    set_target_properties(vf::opencv PROPERTIES INTERFACE_LINK_LIBRARIES opencv_world)
    set(OpenCV_INCLUDE_DIRS "${OPENCV_INCLUDE}")
endif()

# 提供传统变量给仍使用 ${OpenCV_LIBS} 的旧式写法
set(OpenCV_LIBS "vf::opencv")

# =============================================================================
# 公共目标：导出 Include/include 与 Include/lib 给上层构建使用
# =============================================================================
file(MAKE_DIRECTORY "${VF_INCLUDE_DIR}")
file(MAKE_DIRECTORY "${VF_LIB_DIR}")
file(MAKE_DIRECTORY "${VF_BIN_DIR}")

add_subdirectory(FarmeToolBase)
add_subdirectory(FrameLog)
add_subdirectory(FrameCameraBase)
add_subdirectory(FrameImageView)
add_subdirectory(FrameToolRegistry)
add_subdirectory(FrameQueueTool)
add_subdirectory(FrameImageTools)
add_subdirectory(VisionFrame)
