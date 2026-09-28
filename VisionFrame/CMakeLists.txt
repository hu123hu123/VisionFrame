# VisionFrame 主程序（Qt Widgets Application，依赖 3 个内部库 + OpenCV）
# 输出：App/VisionFrame.exe

add_executable(VisionFrame WIN32
    main.cpp
    VisonFrame.cpp
    VisonFrame.h
    VisonFrame.ui
    VisonFrame.qrc
    TaskItem.cpp        TaskItem.h
    GlobalConfig.cpp    GlobalConfig.h
    ProjectConfig.cpp   ProjectConfig.h
    TaskController.cpp TaskController.h
    Graph/TaskGraph.cpp Graph/TaskGraph.h
    NodeEditor/NodeEditorScene.cpp NodeEditor/NodeEditorScene.h
    NodeEditor/NodeEditorView.cpp NodeEditor/NodeEditorView.h
    NodeEditor/NodeGraphicsItem.cpp NodeEditor/NodeGraphicsItem.h
    NodeEditor/PinItem.cpp NodeEditor/PinItem.h
    NodeEditor/WireItem.cpp NodeEditor/WireItem.h
    NodeEditor/ToolPaletteWidget.cpp NodeEditor/ToolPaletteWidget.h
    NodeEditor/ToolFactory.cpp NodeEditor/ToolFactory.h
    Display/ImageDisplayWidget.cpp Display/ImageDisplayWidget.h
)

target_compile_definitions(VisionFrame PRIVATE
    _WINDOWS
    WIN32_LEAN_AND_MEAN
)

target_include_directories(VisionFrame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}           # 使 Graph/、NodeEditor/、Display/ 相对包含可解析
    ${CMAKE_SOURCE_DIR}/FrameQueueTool    # 引用静态库内部头（FrameEnQueueTool.h 等）
    ${VF_INCLUDE_DIR}                     # 引用 Include/include 中的对外头
    ${OpenCV_INCLUDE_DIRS}
)

# 让 TaskItem.h 的 #pragma comment(lib,"FrameToolBase.lib") 能在 Include/lib 找到导入库
target_link_directories(VisionFrame PRIVATE ${VF_LIB_DIR})

target_link_libraries(VisionFrame PRIVATE
    FarmeToolBase
    FrameCameraBase
    FrameQueueTool
    FrameImageTools
    Qt5::Core Qt5::Gui Qt5::Widgets
    ${OpenCV_LIBS}
)

set_target_properties(VisionFrame PROPERTIES
    OUTPUT_NAME       "VisionFrame"
    WIN32_EXECUTABLE  TRUE
    LINK_FLAGS        "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
    RUNTIME_OUTPUT_DIRECTORY "${VF_APP_DIR}"
)

# 业务 DLL 延迟加载：不在启动时解析导入表，而在首次调用时加载
# 这样 main() 中的 SetDllDirectoryW("...\Bin") 先执行，延迟加载的业务 DLL
# 就能从 App/Bin/ 找到，不需要拷贝到 exe 同级目录
if(MSVC)
    target_link_options(VisionFrame PRIVATE
        /DELAYLOAD:FrameToolBase.dll
        /DELAYLOAD:FrameCameraBase.dll
    )
    target_link_libraries(VisionFrame PRIVATE delayimp.lib)
endif()
# 多配置生成器下每配置也覆盖到 App/
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(CONCAT _cfg_prop "RUNTIME_OUTPUT_DIRECTORY_${cfg}")
    set_target_properties(VisionFrame PROPERTIES ${_cfg_prop} "${VF_APP_DIR}")
endforeach()

# =============================================================================
# 构建后拷贝第三方运行期 DLL 到 exe 同级目录（App/）
#   - Qt5Cored / Qt5Core, Qt5Guid / Qt5Gui, Qt5Widgetsd / Qt5Widgets
#   - qwindowsd.dll / qwindows.dll (platforms 插件)
#   - opencv_world4120d.dll / opencv_world4120.dll
# 业务 DLL（FrameToolBase.dll 等）通过 /DELAYLOAD 延迟加载，留在 App/Bin/ 即可。
# 第三方 DLL 是隐式链接，必须在 exe 同级目录，Windows 加载器在 main() 之前解析。
# =============================================================================
set(QT_BIN      "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/bin"                 CACHE PATH "Qt bin 目录")
set(QT_PLUGINS  "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/plugins/platforms"   CACHE PATH "Qt platforms 插件目录")
set(OPENCV_BIN  "${CMAKE_SOURCE_DIR}/Include/opencv4120/x64/bin"        CACHE PATH "OpenCV bin 目录")

# 按配置选择 DLL 名后缀（Debug -> d，Release -> 空）
set(_dll_qt_core       "$<IF:$<CONFIG:Debug>,Qt5Cored.dll,Qt5Core.dll>")
set(_dll_qt_gui        "$<IF:$<CONFIG:Debug>,Qt5Guid.dll,Qt5Gui.dll>")
set(_dll_qt_widgets    "$<IF:$<CONFIG:Debug>,Qt5Widgetsd.dll,Qt5Widgets.dll>")
set(_dll_qt_platform   "$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>")
set(_dll_opencv        "$<IF:$<CONFIG:Debug>,opencv_world4120d.dll,opencv_world4120.dll>")

add_custom_command(TARGET VisionFrame POST_BUILD
    # Qt / OpenCV 运行期 DLL 拷贝到 exe 同级目录
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_core}"      "$<TARGET_FILE_DIR:VisionFrame>/${_dll_qt_core}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_gui}"       "$<TARGET_FILE_DIR:VisionFrame>/${_dll_qt_gui}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_widgets}"   "$<TARGET_FILE_DIR:VisionFrame>/${_dll_qt_widgets}"
    # Qt 平台插件放到 exe 同级的 platforms/
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VisionFrame>/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PLUGINS}/${_dll_qt_platform}" "$<TARGET_FILE_DIR:VisionFrame>/platforms/${_dll_qt_platform}"
    # OpenCV
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENCV_BIN}/${_dll_opencv}"     "$<TARGET_FILE_DIR:VisionFrame>/${_dll_opencv}"
    COMMENT "Copy third-party runtime DLLs to exe directory"
)
