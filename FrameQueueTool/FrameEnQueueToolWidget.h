#pragma once

#include <QWidget>
#include "ui_FrameEnQueueToolWidget.h"

class FrameEnQueueToolWidget : public QWidget
{
	Q_OBJECT

public:
	FrameEnQueueToolWidget(QWidget *parent = nullptr);
	~FrameEnQueueToolWidget();

	QString targetQueueName() const;
	void    setTargetQueueName(const QString& name);

signals:
	void targetQueueNameChanged(const QString& name);

private:
	Ui::FrameEnQueueToolWidgetClass ui;
};
