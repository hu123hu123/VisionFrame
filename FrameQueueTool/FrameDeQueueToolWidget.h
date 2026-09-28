#pragma once

#include <QWidget>
#include "ui_FrameDeQueueToolWidget.h"

class FrameDeQueueToolWidget : public QWidget
{
	Q_OBJECT

public:
	FrameDeQueueToolWidget(QWidget *parent = nullptr);
	~FrameDeQueueToolWidget();

	QString sourceQueueName() const;
	void    setSourceQueueName(const QString& name);

signals:
	void sourceQueueNameChanged(const QString& name);

private:
	Ui::FrameDeQueueToolWidgetClass ui;
};
