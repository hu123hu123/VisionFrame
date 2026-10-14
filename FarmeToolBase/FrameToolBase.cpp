#include "pch.h"
#include "FrameToolBase.h"

// 存活实例计数：构造 +1 / 析构 -1。内存泄漏排查诊断用（配合宿主 diag 日志采样）。
std::atomic<int> FrameToolBase::s_liveCount{ 0 };

FrameToolBase::FrameToolBase()
{
	++s_liveCount;
}

FrameToolBase::~FrameToolBase()
{
	--s_liveCount;
}

int FrameToolBase::liveToolCount()
{
	return s_liveCount.load();
}
