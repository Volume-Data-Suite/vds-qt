#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"

#include <VDTK/VolumeDataHandler.h>

#include "fileio/import_item.h"

namespace VDS
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow(QWidget *parent = Q_NULLPTR);


	public slots:
		void openImportRawDialog();


	private:
		Ui::MainWindowClass ui;

		VDTK::VolumeDataHandler m_vdh;
	};
}


