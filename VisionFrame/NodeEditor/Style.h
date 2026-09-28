#pragma once

#include <QColor>
#include <QString>
#include "FrameToolBase.h"

// 蓝图编辑器统一配色与外观。
namespace NodeStyle
{
inline QColor pinColor(PinType t)
{
    switch (t)
    {
    case PinType::Image:  return QColor(0x4db84d); // 绿
    case PinType::Bool:   return QColor(0xd94b4b); // 红
    case PinType::Int:    return QColor(0x4bc6d9); // 青
    case PinType::Double: return QColor(0x4b7bd9); // 蓝
    case PinType::String: return QColor(0xe0a700); // 橙
    case PinType::Any:    return QColor(0x999999); // 灰
    }
    return QColor(0x999999);
}

inline QColor categoryColor(const QString& category)
{
    if (category == QString::fromUtf8("图像源"))
        return QColor(0x3a7bd5);
    if (category == QString::fromUtf8("图像处理"))
        return QColor(0x7a5cd0);
    return QColor(0x555555);
}

inline QColor background()    { return QColor(0x2b2b2b); }
inline QColor grid()          { return QColor(0x383838); }
inline QColor nodeBody()      { return QColor(0x3c3c3c); }
inline QColor nodeBorder()    { return QColor(0x1a1a1a); }
inline QColor text()          { return QColor(0xdddddd); }
inline QColor titleBarText()  { return QColor(0xffffff); }
inline QColor activeBorder()  { return QColor(0x2dff8c); }
inline QColor selectBorder()  { return QColor(0x2d8cff); }
}