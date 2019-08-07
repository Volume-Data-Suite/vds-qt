#include "main_window.h"

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

int main(int argc, char* argv[]) {

    // Set OpenGL format
    QSurfaceFormat fmt;
    fmt.setMajorVersion(4);
    fmt.setMinorVersion(3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
    fmt.setSwapInterval(0);
#ifdef _DEBUG
    fmt.setOption(QSurfaceFormat::DebugContext);
#endif
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication a(argc, argv);

    VDS::MainWindow w;
    w.show();

    return a.exec();
}