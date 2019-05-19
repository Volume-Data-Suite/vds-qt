#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"

#include <VDTK/VolumeDataHandler.h>



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

	void openImportRawDialog();

private:
	Ui::MainWindowClass ui;

	VDTK::VolumeDataHandler m_vdh;
};
