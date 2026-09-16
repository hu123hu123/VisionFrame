# FrameCameraBase 基础接口库（动态库，依赖 OpenCV）
# 输出：App/Bin/FrameCameraBase.dll，Include/lib/FrameCameraBase.lib，Include/include/FrameCameraBase.h

add_library(FrameCameraBase SHARED
    dllmain.cpp
    FrameCameraBase.cpp
    pch.cpp
    pch.h
    framework.h
    FrameCameraBase.h
)

target_precompile_headers(FrameCameraBase PRIVATE pch.h)

target_compile_definitions(FrameCameraBase PRIVATE
    FRAMECAMERABASE_EXPORTS
    _WINDOWS
    _USRDLL
    WIN32_LEAN_AND_MEAN
)

target_include_directories(FrameCameraBase PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(FrameCameraBase PRIVATE ${OpenCV_LIBS})

set_target_properties(FrameCameraBase PROPERTIES
    OUTPUT_NAME           "FrameCameraBase"
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    LINK_FLAGS            "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
)

add_custom_command(TARGET FrameCameraBase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/FrameCameraBase.h"
        "${VF_INCLUDE_DIR}/FrameCameraBase.h"
    COMMENT "Copy FrameCameraBase.h -> Include/include"
)
