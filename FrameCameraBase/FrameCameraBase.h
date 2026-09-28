#pragma once
#include "opencv2/opencv.hpp"

#ifdef FRAMECAMERABASE_EXPORTS
#define FRAMECAMERABASE_API __declspec(dllexport)
#else
#define FRAMECAMERABASE_API __declspec(dllimport)
#endif
class FRAMECAMERABASE_API FrameCameraBase
{
public:
	virtual void GetImge(cv::Mat& img)const = 0;
};

