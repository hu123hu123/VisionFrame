#include "ToolFactory.h"
#include "ImageSourceTool.h"
#include "ImageSourceToolWidget.h"
#include "ImageBlurTool.h"
#include "ImageBlurToolWidget.h"
#include "ImageGrayscaleTool.h"
#include <QObject>

ToolFactory& ToolFactory::instance()
{
    static ToolFactory s;
    return s;
}

void ToolFactory::registerTool(const ToolEntry& e)
{
    m_tools.append(e);
}

const QVector<ToolEntry>& ToolFactory::tools() const
{
    return m_tools;
}

const ToolEntry* ToolFactory::find(const QString& typeId) const
{
    for (const auto& e : m_tools)
        if (e.typeId == typeId)
            return &e;
    return nullptr;
}

FrameToolBase* ToolFactory::create(const QString& typeId) const
{
    const ToolEntry* e = find(typeId);
    return (e && e->create) ? e->create() : nullptr;
}

QVector<QString> ToolFactory::categories() const
{
    QVector<QString> out;
    for (const auto& e : m_tools)
        if (!out.contains(e.category))
            out.append(e.category);
    return out;
}

QString ToolFactory::displayName(const QString& typeId) const
{
    const ToolEntry* e = find(typeId);
    return e ? e->displayName : typeId;
}

void registerBuiltinTools()
{
    ToolFactory& f = ToolFactory::instance();

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
    f.registerTool(src);

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
    f.registerTool(blur);

    ToolEntry gray;
    gray.typeId = "ImageGrayscaleTool";
    gray.displayName = QString::fromUtf8("灰度");
    gray.category = QString::fromUtf8("图像处理");
    gray.create = []() -> FrameToolBase* { return new ImageGrayscaleTool(); };
    gray.createWidget = nullptr;
    f.registerTool(gray);
}