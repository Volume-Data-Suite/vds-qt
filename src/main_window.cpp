#include "main_window.h"

#include "fileio/import_raw_3D_dialog.h"

#include "common/vdtk_helper_functions.h"

#include <QDialog>

namespace VDS
{
	MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
	{
		ui.setupUi(this);

		setWindowTitle(QString("Volume Data Suite"));

		connect(ui.actionImport_Raw, &QAction::triggered, this, &MainWindow::openImportRawDialog);
	}

	void MainWindow::openImportRawDialog()
	{

		DialogImportRAW3D dialog;
		dialog.show();
		
		if (dialog.exec() != QDialog::Accepted) {
			// Raw Import got canceled by user
			return;
		}

		ImportItemRaw item = dialog.getImportItem();

		m_vdh.importRawFile(item.getFilePath(), item.getBitsPerVoxel(), Helper::QVector3DToVolumeSize(item.getSize()), Helper::QVector3DToVolumeSpacing(item.getSpacing()));

	}
}


