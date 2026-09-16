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
)

target_compile_definitions(VisionFrame PRIVATE
    _WINDOWS
    WIN32_LEAN_AND_MEAN
)

target_include_directories(VisionFrame PRIVATE
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
    Qt5::Core Qt5::Gui Qt5::Widgets
    ${OpenCV_LIBS}
)

set_target_properties(VisionFrame PROPERTIES
    OUTPUT_NAME       "VisionFrame"
    WIN32_EXECUTABLE  TRUE
    LINK_FLAGS        "/SUBSYSTEM:WINDOWS /MANIFESTUAC:NO"
)

# =============================================================================
# 构建后拷贝运行期 DLL 到 App/Bin（与原 PostBuildEvent 一致）
#   - Qt5Cored / Qt5Core, Qt5Guid / Qt5Gui, Qt5Widgetsd / Qt5Widgets
#   - qwindowsd.dll / qwindows.dll (platforms 插件)
#   - opencv_world4120d.dll / opencv_world4120.dll
# 使用 $<CONFIG:Debug> 区分 Debug/Release：Debug 拷贝带 d 后缀的版本。
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
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VisionFrame>/Bin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VisionFrame>/Bin/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_core}"     "$<TARGET_FILE_DIR:VisionFrame>/Bin/${_dll_qt_core}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_gui}"      "$<TARGET_FILE_DIR:VisionFrame>/Bin/${_dll_qt_gui}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN}/${_dll_qt_widgets}"  "$<TARGET_FILE_DIR:VisionFrame>/Bin/${_dll_qt_widgets}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PLUGINS}/${_dll_qt_platform}" "$<TARGET_FILE_DIR:VisionFrame>/Bin/platforms/${_dll_qt_platform}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENCV_BIN}/${_dll_opencv}"  "$<TARGET_FILE_DIR:VisionFrame>/Bin/${_dll_opencv}"
    COMMENT "Copy Qt and OpenCV runtime DLLs to App/Bin"
)
