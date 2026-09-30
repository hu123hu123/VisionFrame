#pragma once

#ifdef FRAMEIMAGEVIEW_EXPORTS
#define FRAMEIMAGEVIEW_API __declspec(dllexport)
#else
#define FRAMEIMAGEVIEW_API __declspec(dllimport)
#endif