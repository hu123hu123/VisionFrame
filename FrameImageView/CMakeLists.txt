# FrameImageView 可复用图像显示/图形绘制库（动态库，依赖 Qt + OpenCV）
# 输出：App/Bin/FrameImageView.dll，Include/lib/FrameImageView.lib，
#       公共头复制到 Include/include（供主程序与工具库复用）

add_library(FrameImageView SHARED
    dllmain.cpp
    FrameImageViewExport.h
    ImageViewWidget.cpp ImageViewWidget.h
    RectRoiItem.cpp RectRoiItem.h
    RoiImageViewWidget.cpp RoiImageViewWidget.h
)

target_compile_definitions(FrameImageView PRIVATE
    FRAMEIMAGEVIEW_EXPORTS
    _WINDOWS
    _USRDLL
    WIN32_LEAN_AND_MEAN
)

# PUBLIC：让下游（VisionFrame / FrameImageTools 等）直接 #include "ImageViewWidget.h"
# 并能拿到 OpenCV 头路径与 Qt/OpenCV 链接依赖
target_include_directories(FrameImageView PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(FrameImageView PUBLIC
    Qt5::Core Qt5::Gui Qt5::Widgets
    ${OpenCV_LIBS}
)

set_target_properties(FrameImageView PROPERTIES
    OUTPUT_NAME           "FrameImageView"
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
    LINK_FLAGS            "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
)

foreach(_h FrameImageViewExport.h ImageViewWidget.h RectRoiItem.h RoiImageViewWidget.h)
    add_custom_command(TARGET FrameImageView POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${_h}"
            "${VF_INCLUDE_DIR}/${_h}"
        COMMENT "Copy ${_h} -> Include/include"
    )
endforeach()