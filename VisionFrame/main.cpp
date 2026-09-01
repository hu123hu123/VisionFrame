#include "VisonFrame.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    VisionFrame window;
    window.show();
    return app.exec();
}
