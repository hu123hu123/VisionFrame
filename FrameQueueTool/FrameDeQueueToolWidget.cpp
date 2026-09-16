#include "FrameDeQueueToolWidget.h"

FrameDeQueueToolWidget::FrameDeQueueToolWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.sourceQueueNameLineEdit, &QLineEdit::textChanged,
		this, &FrameDeQueueToolWidget::sourceQueueNameChanged);
}

FrameDeQueueToolWidget::~FrameDeQueueToolWidget()
{
}

QString FrameDeQueueToolWidget::sourceQueueName() const
{
	return ui.sourceQueueNameLineEdit->text();
}

void FrameDeQueueToolWidget::setSourceQueueName(const QString& name)
{
	ui.sourceQueueNameLineEdit->setText(name);
}
