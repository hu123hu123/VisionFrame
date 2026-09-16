#pragma once

#include <QThread>
#include <QString>
#include "FrameToolBase.h"
#include "FrameCameraBase.h"
#include "FrameDataQueue.h"
#include "opencv2/opencv.hpp"
#include <any>
#include <memory>

#pragma comment(lib, "FrameToolBase.lib")
#pragma comment(lib, "FrameCameraBase.lib")
// FrameQueueTool.lib 通过 VisonFrame.vcxproj 的 ProjectReference 链接，
// 该 lib 不在 Include\lib 中，故不在此用 #pragma。

// 一个 TaskItem = 一个任务 = 一个线程(QThread)。
// 内部维护工具列表 m_tools，doTask() 顺序执行每个工具。
// 跨线程数据传递：本任务队列 m_dataQueue 以任务名注册到 FrameDataQueueRegistry，
// 上游任务的 FrameEnQueueTool 指定本任务名即可把数据投递过来；
// 本任务的 FrameDeQueueTool 在 doTask() 中阻塞出队，作为线程启用条件。
class TaskItem : public QThread
{
	Q_OBJECT

public:
	TaskItem(QObject *parent = nullptr);
	~TaskItem();

	void StartTask();
	void StopTask();

	// 任务名，同时是本任务队列在注册表中的键。
	QString taskName() const;
	void    setTaskName(const QString& name);

	// 工具列表管理（public，便于外部装配任务链）。
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

	// 任务级数据入队/出队（线程安全，委托给 m_dataQueue）。
	// 上游 FrameEnQueueTool 跨线程调用 enqueueData 即可把数据送入本任务。
	template<typename T>
	void enqueueData(T&& data);
	std::any dequeueData();

	// 暴露本任务队列（shared_ptr 共享所有权）。
	std::shared_ptr<FrameDataQueue> dataQueue() const;

private:
	void run() override;
	void doTask();
	void registerQueue();

private:
	QString                                   m_taskName;
	QVector<FrameToolBase*>                    m_tools;
	QVector<FrameCameraBase*>                  m_cameras;
	QVector<cv::Mat>                           m_imgs;
	std::shared_ptr<FrameDataQueue>            m_dataQueue;
	std::atomic<bool>                          m_stop{ false };
};


template<typename T>
inline void TaskItem::enqueueData(T&& data)
{
	if (m_dataQueue)
		m_dataQueue->enqueue(std::any(std::forward<T>(data)));
}