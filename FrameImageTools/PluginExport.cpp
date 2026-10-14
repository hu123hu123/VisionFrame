#include "ToolFactory.h"
#include "FrameToolPlugin.h"

#include "ImageSourceTool.h"
#include "ImageSourceToolWidget.h"
#include "ImageBlurTool.h"
#include "ImageBlurToolWidget.h"
#include "ImageGrayscaleTool.h"
#include "TemplateMatchTool.h"
#include "TemplateMatchToolWidget.h"
#include "AffineTransformTool.h"
#include "AffineTransformToolWidget.h"
#include "ImageSaveTool.h"
#include "ImageSaveToolWidget.h"
#include <QObject>

// =============================================================================
// FrameImageTools 工具插件导出：把本插件全部图像工具注册进宿主 ToolFactory。
// 注册内容由原宿主 registerBuiltinTools() 迁移而来，工具行为与 typeId 不变。
// =============================================================================

VF_PLUGIN_EXPORT int vfPluginApiVersion()
{
    return VF_PLUGIN_API_VERSION;
}

VF_PLUGIN_EXPORT void vfRegisterTools(ToolFactory* f)
{
    if (!f)
        return;

    ToolEntry src;
    src.typeId = "ImageSourceTool";
    src.displayName = QString::fromUtf8("图像源");
    src.category = QString::fromUtf8("图像源");
    src.create = []() -> FrameToolBase* { return new ImageSourceTool(); };
    src.createWidget = [](FrameToolBase* t) -> class QWidget* {
        auto* tool = static_cast<ImageSourceTool*>(t);
        auto* w = new ImageSourceToolWidget();
        w->setImagePath(QString::fromStdString(tool->imagePath()));
        QObject::connect(w, &ImageSourceToolWidget::imagePathChanged, w,
            [tool](const QString& p) { tool->setImagePath(p.toStdString()); });
        return w;
    };
    f->registerTool(src);

    ToolEntry blur;
    blur.typeId = "ImageBlurTool";
    blur.displayName = QString::fromUtf8("高斯模糊");
    blur.category = QString::fromUtf8("图像处理");
    blur.create = []() -> FrameToolBase* { return new ImageBlurTool(); };
    blur.createWidget = [](FrameToolBase* t) -> class QWidget* {
        auto* tool = static_cast<ImageBlurTool*>(t);
        auto* w = new ImageBlurToolWidget();
        w->setKernelSize(tool->kernelSize());
        QObject::connect(w, &ImageBlurToolWidget::kernelSizeChanged, w,
            [tool](int k) { tool->setKernelSize(k); });
        return w;
    };
    f->registerTool(blur);

    ToolEntry gray;
    gray.typeId = "ImageGrayscaleTool";
    gray.displayName = QString::fromUtf8("灰度二值化");
    gray.category = QString::fromUtf8("图像处理");
    gray.create = []() -> FrameToolBase* { return new ImageGrayscaleTool(); };
    gray.createWidget = nullptr;
    f->registerTool(gray);

    ToolEntry match;
    match.typeId = "TemplateMatchTool";
    match.displayName = QString::fromUtf8("模板匹配");
    match.category = QString::fromUtf8("图像处理");
    match.create = []() -> FrameToolBase* { return new TemplateMatchTool(); };
    match.createWidget = [](FrameToolBase* t) -> class QWidget* {
        auto* tool = static_cast<TemplateMatchTool*>(t);
        auto* w = new TemplateMatchToolWidget();
        w->setMethod(tool->method());
        w->setThreshold(tool->threshold());
        if (tool->hasTemplate())
            w->setTemplate(tool->templateImage());
        QObject::connect(w, &TemplateMatchToolWidget::methodChanged, w,
            [tool](int m) { tool->setMethod(m); });
        QObject::connect(w, &TemplateMatchToolWidget::thresholdChanged, w,
            [tool](double v) { tool->setThreshold(v); });
        QObject::connect(w, &TemplateMatchToolWidget::templateTrained, w,
            [tool](const cv::Mat& tpl) { tool->setTemplate(tpl); });
        return w;
    };
    f->registerTool(match);

    ToolEntry affine;
    affine.typeId = "AffineTransformTool";
    affine.displayName = QString::fromUtf8("仿射变换");
    affine.category = QString::fromUtf8("图像处理");
    affine.create = []() -> FrameToolBase* { return new AffineTransformTool(); };
    affine.createWidget = [](FrameToolBase* t) -> class QWidget* {
        auto* tool = static_cast<AffineTransformTool*>(t);
        auto* w = new AffineTransformToolWidget();
        w->setAngle(tool->angle());
        w->setScale(tool->scale());
        w->setTransX(tool->transX());
        w->setTransY(tool->transY());
        w->setInterpolation(tool->interpolation());
        QObject::connect(w, &AffineTransformToolWidget::changed, w,
            [tool, w]() {
                tool->setAngle(w->angle());
                tool->setScale(w->scale());
                tool->setTransX(w->transX());
                tool->setTransY(w->transY());
                tool->setInterpolation(w->interpolation());
            });
        return w;
    };
    f->registerTool(affine);

    ToolEntry save;
    save.typeId = "ImageSaveTool";
    save.displayName = QString::fromUtf8("图像保存");
    save.category = QString::fromUtf8("图像处理");
    save.create = []() -> FrameToolBase* { return new ImageSaveTool(); };
    save.createWidget = [](FrameToolBase* t) -> class QWidget* {
        auto* tool = static_cast<ImageSaveTool*>(t);
        auto* w = new ImageSaveToolWidget();
        w->setSavePath(tool->savePath());
        QObject::connect(w, &ImageSaveToolWidget::changed, w,
            [tool, w]() { tool->setSavePath(w->savePath()); });
        return w;
    };
    f->registerTool(save);
}
