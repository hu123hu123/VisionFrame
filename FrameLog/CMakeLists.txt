# FrameLog 轻量日志核心库（动态库，无 Qt 依赖）
# 输出：App/Bin/FrameLog.dll，Include/lib/FrameLog.lib，Include/include/FrameLog.h

add_library(FrameLog SHARED
    dllmain.cpp
    FrameLog.cpp        FrameLog.h
    CrashReporter.cpp   CrashReporter.h
    OsCompat.h
    pch.cpp
    pch.h
    framework.h
)

# 预编译头
target_precompile_headers(FrameLog PRIVATE pch.h)

target_compile_definitions(FrameLog PRIVATE FRAMELOG_EXPORTS)
if(WIN32)
    target_compile_definitions(FrameLog PRIVATE
        _WINDOWS
        _USRDLL
        WIN32_LEAN_AND_MEAN
    )
endif()

# 让下游在构建期可直接 include FrameLog.h（无需等待头复制步骤）
target_include_directories(FrameLog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(FrameLog PROPERTIES
    OUTPUT_NAME           "FrameLog"
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
)
if(MSVC)
    set_target_properties(FrameLog PROPERTIES
        LINK_FLAGS            "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
    )
endif()

# 构建后把公共头复制到 Include/include
add_custom_command(TARGET FrameLog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/FrameLog.h"
        "${VF_INCLUDE_DIR}/FrameLog.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/CrashReporter.h"
        "${VF_INCLUDE_DIR}/CrashReporter.h"
    COMMENT "Copy FrameLog.h/CrashReporter.h -> Include/include"
)