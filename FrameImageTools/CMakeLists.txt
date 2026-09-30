# FrameImageTools 静态库（OpenCV 图像工具，依赖 FrameToolBase + Qt + OpenCV）
# 输出：Include/lib/FrameImageTools.lib，公共头复制到 Include/include

add_library(FrameImageTools STATIC
    ImageSourceTool.cpp  ImageSourceTool.h
    ImageSourceToolWidget.cpp  ImageSourceToolWidget.h
    ImageBlurTool.cpp  ImageBlurTool.h
    ImageBlurToolWidget.cpp  ImageBlurToolWidget.h
    ImageGrayscaleTool.cpp  ImageGrayscaleTool.h
    TemplateMatchTool.cpp  TemplateMatchTool.h
    TemplateMatchToolWidget.cpp  TemplateMatchToolWidget.h  TemplateMatchToolWidget.ui
    AffineTransformTool.cpp  AffineTransformTool.h
    AffineTransformToolWidget.cpp  AffineTransformToolWidget.h  AffineTransformToolWidget.ui
)

target_compile_definitions(FrameImageTools PRIVATE BUILD_STATIC)

target_include_directories(FrameImageTools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VF_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/FarmeToolBase
    ${CMAKE_SOURCE_DIR}/FrameImageView    # RoiImageViewWidget.h 等（模板训练预览）
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(FrameImageTools PUBLIC
    FarmeToolBase
    FrameImageView
    Qt5::Core Qt5::Gui Qt5::Widgets
    ${OpenCV_LIBS}
)

set_target_properties(FrameImageTools PROPERTIES OUTPUT_NAME "FrameImageTools")

# 构建后把公共头复制到 Include/include
set(FIT_PUBLIC_HEADERS
    ImageSourceTool.h
    ImageBlurTool.h
    ImageGrayscaleTool.h
    TemplateMatchTool.h
    AffineTransformTool.h
)
foreach(hdr ${FIT_PUBLIC_HEADERS})
    add_custom_command(TARGET FrameImageTools POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${hdr}"
            "${VF_INCLUDE_DIR}/${hdr}"
        COMMENT "Copy ${hdr} -> Include/include"
    )
endforeach()