# FrameQueueTool 静态库（依赖 Qt 与 FrameToolBase 接口头）
# 输出：Include/lib/FrameQueueTool.lib，公共头复制到 Include/include
# AUTOMOC/AUTOUIC 由顶层 CMakeLists 全局开启

add_library(FrameQueueTool STATIC
    FrameDataQueue.cpp          FrameDataQueue.h
    FrameDataQueueRegistry.cpp  FrameDataQueueRegistry.h
    FrameEnQueueTool.cpp        FrameEnQueueTool.h
    FrameEnQueueToolWidget.cpp  FrameEnQueueToolWidget.h  FrameEnQueueToolWidget.ui
    FrameDeQueueTool.cpp        FrameDeQueueTool.h
    FrameDeQueueToolWidget.cpp  FrameDeQueueToolWidget.h  FrameDeQueueToolWidget.ui
)

target_compile_definitions(FrameQueueTool PRIVATE
    FRAMEQUEUETOOL_LIB
    BUILD_STATIC
)

# FrameQueueTool 编译时按 C++17（与原 vcxproj 一致）；上层 VisionFrame 仍可用 C++20
set_target_properties(FrameQueueTool PROPERTIES CXX_STANDARD 17)

target_include_directories(FrameQueueTool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}      # 便于编译单元内部互引
    ${VF_INCLUDE_DIR}                # 引用 Include/include 中的 FrameToolBase.h
    ${CMAKE_SOURCE_DIR}/FarmeToolBase # 直接引用源目录的 FrameToolBase.h（避免依赖 .h 复制步骤）
)

target_link_libraries(FrameQueueTool PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)

set_target_properties(FrameQueueTool PROPERTIES
    OUTPUT_NAME "FrameQueueTool"
)

# 构建后把公共头复制到 Include/include（与原 PostBuildEvent 一致）
set(FQT_PUBLIC_HEADERS
    FrameDataQueue.h
    FrameDataQueueRegistry.h
    FrameEnQueueTool.h
    FrameDeQueueTool.h
)
foreach(hdr ${FQT_PUBLIC_HEADERS})
    add_custom_command(TARGET FrameQueueTool POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${hdr}"
            "${VF_INCLUDE_DIR}/${hdr}"
        COMMENT "Copy ${hdr} -> Include/include"
    )
endforeach()
