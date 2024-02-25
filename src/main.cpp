#include "main_window.h"

#include <QSurfaceFormat>
#include <QThread>
#include <QFile>
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

    QApplication app(argc, argv);

    QFile file(":/stylesheets/stylesheets/style.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);

    QThread::currentThread()->setObjectName("Main Thread");

    VDS::MainWindow window;
    window.show();

    return app.exec();
}