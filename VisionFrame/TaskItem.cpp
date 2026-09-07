#include "TaskItem.h"

TaskItem::TaskItem(QObject *parent)
	: QThread(parent)
{}

TaskItem::~TaskItem()
{}

void TaskItem::StartTask()
{
	if(!this->isRunning())
		this->start();
}

void TaskItem::StopTask()
{
	this->requestInterruption();
	this->wait();
}

void TaskItem::run()
{
	while (this->isRunning())
	{
		doTask();
	}
}

void TaskItem::doTask()
{

}

void TaskItem::addTool(FrameToolBase* tool)
{
}

