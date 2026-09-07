#pragma once

#include <QThread>
#include "FrameToolBase.h"
#include "FrameCameraBase.h"
#include "opencv2/opencv.hpp"

#pragma comment(lib, "FrameToolBase.lib")
#pragma comment(lib, "FrameCameraBase.lib")
class TaskItem  : public QThread
{
	Q_OBJECT

public:
	TaskItem(QObject *parent);
	~TaskItem();
	void StartTask();
	void StopTask();
	
private:
	void run() override;
	void doTask();
	void addTool(FrameToolBase* tool);
	void addToolAt(int index, FrameToolBase* tool);
	void removeToolAt(int index);
	void removeTool(QString toolName);
	FrameToolBase* getTool(QString toolName);
	FrameToolBase* getToolAt(int index);
	QVector<FrameToolBase*> getTools();
	void setTools(QVector<FrameToolBase*> tools);
	void clearTools();
	void addCamera(FrameCameraBase* camera);
	void addCameraAt(int index, FrameCameraBase* camera);
	void removeCameraAt(int index);
	//数据入队与出队
	void enqueueData();
	void dequeueData();
private:
	QVector<FrameToolBase*> m_tools;
	QVector<FrameCameraBase*> m_cameras;
	QVector<cv::Mat> m_imgs;
};

