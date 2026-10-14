# FrameImageTools 工具插件 DLL（OpenCV 图像工具，依赖 FrameToolBase + FrameToolRegistry + Qt + OpenCV）
# 输出：App/Bin/plugins/FrameImageTools.dll（工具插件目录，宿主启动时扫描加载）
# 插件契约：导出 vfPluginApiVersion / vfRegisterTools，经 ToolFactory 注册本插件全部工具

add_library(FrameImageTools SHARED
    PluginExport.cpp
    ImageSourceTool.cpp  ImageSourceTool.h
    ImageSourceToolWidget.cpp  ImageSourceToolWidget.h
    ImageBlurTool.cpp  ImageBlurTool.h
    ImageBlurToolWidget.cpp  ImageBlurToolWidget.h
    ImageGrayscaleTool.cpp  ImageGrayscaleTool.h
    TemplateMatchTool.cpp  TemplateMatchTool.h
    TemplateMatchToolWidget.cpp  TemplateMatchToolWidget.h  TemplateMatchToolWidget.ui
    AffineTransformTool.cpp  AffineTransformTool.h
    AffineTransformToolWidget.cpp  AffineTransformToolWidget.h  AffineTransformToolWidget.ui
    ImageSaveTool.cpp  ImageSaveTool.h
    ImageSaveToolWidget.cpp  ImageSaveToolWidget.h  ImageSaveToolWidget.ui
)

target_include_directories(FrameImageTools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${VF_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/FarmeToolBase
    ${CMAKE_SOURCE_DIR}/FrameImageView    # RoiImageViewWidget.h 等（模板训练预览）
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(FrameImageTools PUBLIC
    FarmeToolBase
    FrameToolRegistry
    FrameImageView
    FrameLog
    Qt5::Core Qt5::Gui Qt5::Widgets
    ${OpenCV_LIBS}
)

set_target_properties(FrameImageTools PROPERTIES OUTPUT_NAME "FrameImageTools")

# 插件输出到 App/Bin/plugins（覆盖全局 App/Bin 默认值，含多配置生成器 per-config 属性）
# Windows 下 .dll 属 RUNTIME 产物；Linux 下 .so 属 LIBRARY 产物，两者都指向 plugins
set_target_properties(FrameImageTools PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VF_BIN_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${VF_BIN_DIR}/plugins"
)
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(FrameImageTools PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${VF_BIN_DIR}/plugins"
        LIBRARY_OUTPUT_DIRECTORY_${_cfg} "${VF_BIN_DIR}/plugins"
    )
endforeach()

# 构建后把公共头复制到 Include/include（对外 SDK：第三方工具插件开发者引用）
set(FIT_PUBLIC_HEADERS
    ImageSourceTool.h
    ImageBlurTool.h
    ImageGrayscaleTool.h
    TemplateMatchTool.h
    AffineTransformTool.h
    ImageSaveTool.h
)
foreach(hdr ${FIT_PUBLIC_HEADERS})
    add_custom_command(TARGET FrameImageTools POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${hdr}"
            "${VF_INCLUDE_DIR}/${hdr}"
        COMMENT "Copy ${hdr} -> Include/include"
    )
endforeach()
