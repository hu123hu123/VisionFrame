# FrameToolRegistry 工具注册表动态库（工具插件契约的共享锚点）
# 输出：App/Bin/FrameToolRegistry.dll；VisionFrame POST_BUILD 拷贝到 exe 同级目录
# （宿主隐式链接，启动期由加载器在 exe 目录解析；插件经应用目录搜索同样命中）

add_library(FrameToolRegistry SHARED
    FrameToolRegistryExport.h
    FrameToolPlugin.h
    ToolFactory.cpp  ToolFactory.h
)

target_compile_definitions(FrameToolRegistry PRIVATE FRAMETOOLREGISTRY_EXPORTS)

# 仅依赖 Qt Core（QString/QVector）；FrameToolBase/QWidget 仅前向声明，不参与链接
target_link_libraries(FrameToolRegistry PUBLIC Qt5::Core)

target_include_directories(FrameToolRegistry PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(FrameToolRegistry PROPERTIES
    OUTPUT_NAME "FrameToolRegistry"
)
