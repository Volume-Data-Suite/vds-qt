#include "main_window.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionImport_Raw, &QAction::triggered, this, &MainWindow::openImportRawDialog);
}

void MainWindow::openImportRawDialog()
{

}
