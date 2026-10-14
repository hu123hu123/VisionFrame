#pragma once

#if defined(_WIN32)
  #ifdef FRAMEIMAGEVIEW_EXPORTS
  #define FRAMEIMAGEVIEW_API __declspec(dllexport)
  #else
  #define FRAMEIMAGEVIEW_API __declspec(dllimport)
  #endif
#else
  #define FRAMEIMAGEVIEW_API __attribute__((visibility("default")))
#endif