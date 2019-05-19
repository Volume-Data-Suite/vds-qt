#include "main_window.h"

#include <QtWidgets/QApplication>


int main(int argc, char *argv[]) {

	// Set OpenGL format
	QSurfaceFormat fmt;
	fmt.setMajorVersion(4);
	fmt.setMinorVersion(3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	// TODO: expose a setting for buffer value (ie default/single/double/triple)
	fmt.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
#ifdef QT_DEBUG
	fmt.setOption(QSurfaceFormat::DebugContext);
#endif
	QSurfaceFormat::setDefaultFormat(fmt);



	QApplication a(argc, argv);

	VDS::MainWindow w;
	w.show();

	return a.exec();
}