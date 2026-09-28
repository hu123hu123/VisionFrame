#include "FrameEnQueueToolWidget.h"

FrameEnQueueToolWidget::FrameEnQueueToolWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.targetQueueNameLineEdit, &QLineEdit::textChanged,
		this, &FrameEnQueueToolWidget::targetQueueNameChanged);
}

FrameEnQueueToolWidget::~FrameEnQueueToolWidget()
{
}

QString FrameEnQueueToolWidget::targetQueueName() const
{
	return ui.targetQueueNameLineEdit->text();
}

void FrameEnQueueToolWidget::setTargetQueueName(const QString& name)
{
	ui.targetQueueNameLineEdit->setText(name);
}
