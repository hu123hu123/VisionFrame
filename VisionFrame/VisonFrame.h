#pragma once

#include <QtWidgets/QWidget>
#include "ui_VisonFrame.h"

class VisionFrame : public QWidget
{
    Q_OBJECT

public:
    VisionFrame(QWidget *parent = nullptr);
    ~VisionFrame();

private:
    Ui::VisonFrameClass ui;
};

