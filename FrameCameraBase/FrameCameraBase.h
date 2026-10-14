#pragma once
#include "opencv2/opencv.hpp"

#if defined(_WIN32)
  #ifdef FRAMECAMERABASE_EXPORTS
  #define FRAMECAMERABASE_API __declspec(dllexport)
  #else
  #define FRAMECAMERABASE_API __declspec(dllimport)
  #endif
#else
  #define FRAMECAMERABASE_API __attribute__((visibility("default")))
#endif
class FRAMECAMERABASE_API FrameCameraBase
{
public:
	virtual void GetImge(cv::Mat& img)const = 0;
};

