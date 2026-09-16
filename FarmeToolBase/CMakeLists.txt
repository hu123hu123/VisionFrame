# FrameToolBase 基础接口库（动态库）
# 输出：App/Bin/FarmeToolBase.dll，Include/lib/FarmeToolBase.lib，Include/include/FrameToolBase.h

add_library(FarmeToolBase SHARED
    dllmain.cpp
    FrameToolBase.cpp
    pch.cpp
    pch.h
    framework.h
    FrameToolBase.h
)

# 预编译头
target_precompile_headers(FarmeToolBase PRIVATE pch.h)

target_compile_definitions(FarmeToolBase PRIVATE
    FARMETOOLBASE_EXPORTS
    _WINDOWS
    _USRDLL
    WIN32_LEAN_AND_MEAN
)

set_target_properties(FarmeToolBase PROPERTIES
    OUTPUT_NAME           "FrameToolBase"   # 与原 ProjectName=FrameToolBase 及 TaskItem.h 的 #pragma comment(lib,"FrameToolBase.lib") 一致
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    LINK_FLAGS            "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
)

# 构建后把公共头复制到 Include/include（与原 PostBuildEvent 一致）
add_custom_command(TARGET FarmeToolBase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/FrameToolBase.h"
        "${VF_INCLUDE_DIR}/FrameToolBase.h"
    COMMENT "Copy FrameToolBase.h -> Include/include"
)
