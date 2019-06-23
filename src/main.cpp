#include "main_window.h"

#include <QtWidgets/QApplication>
#include <QSurfaceFormat>


int main(int argc, char *argv[]) {
	
	// Set OpenGL format
	QSurfaceFormat fmt;
	fmt.setMajorVersion(4);
	fmt.setMinorVersion(3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	// TODO: expose a setting for buffer value (ie single/double/triple) for V-Sync
	fmt.setSwapBehavior(QSurfaceFormat::SingleBuffer);
	fmt.setSwapInterval(1);
#ifdef _DEBUG
	fmt.setOption(QSurfaceFormat::DebugContext);
#endif
	QSurfaceFormat::setDefaultFormat(fmt);



	QApplication a(argc, argv);

	VDS::MainWindow w;
	w.show();

	return a.exec();
}