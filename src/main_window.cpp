#include "main_window.h"

#include "fileio/import_raw_3D_dialog.h"

#include "common/vdtk_helper_functions.h"

#include <QDialog>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSlider>
#include <QGroupBox>



namespace VDS
{
	MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
	{
		ui.setupUi(this);

		setWindowTitle(QString("Volume Data Suite"));

		setupFileMenu();

		// as long as its not functional
		ui.groupBoxSliceView->hide();

		// connect debug infos
		connect(ui.volumeViewWidget, &VolumeViewGL::updateFrametime,
			this, &MainWindow::updateFrametime);
		connect(ui.checkBoxRenderLoop, &QCheckBox::stateChanged,
			ui.volumeViewWidget, &VolumeViewGL::setRenderLoop);

		// connect sample step length
		connect(ui.doubleSpinBoxSampleRate, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			ui.volumeViewWidget, &VolumeViewGL::setSampleStepLength);
		connect(ui.comboBoxSampleRateRecommended, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			ui.volumeViewWidget, &VolumeViewGL::setRecommendedSampleStepLength);
		connect(ui.volumeViewWidget, &VolumeViewGL::updateSampleStepLength,
			ui.doubleSpinBoxSampleRate, &QDoubleSpinBox::setValue);

		// connect threshold
		connect(ui.horizontalSliderThreshold, &QSlider::valueChanged,
			this, &MainWindow::updateThresholdFromSlider);
		
		// connect raycast method
		connect(ui.comboBoxShaderMethod, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			ui.volumeViewWidget, &VolumeViewGL::setRaycastMethod);

		// connect value window
		connect(ui.groupBoxApplyWindow, &QGroupBox::toggled,
			ui.volumeViewWidget, &VolumeViewGL::applyValueWindow);
		connect(ui.spinBoxApplyWindowValueWindowWidth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			ui.volumeViewWidget, &VolumeViewGL::updateValueWindowWidth);
		connect(ui.spinBoxApplyWindowValueWindowCenter, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			ui.volumeViewWidget, &VolumeViewGL::updateValueWindowCenter);
		connect(ui.spinBoxApplyWindowValueWindowOffset, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			ui.volumeViewWidget, &VolumeViewGL::updateValueWindowOffset);
	}

	void MainWindow::openImportRawDialog()
	{
		DialogImportRAW3D dialog;
		dialog.show();
		
		if (dialog.exec() != QDialog::Accepted) {
			// Raw Import got canceled by user
			return;
		}

		const ImportItemRaw item3D = dialog.getImportItem();
		importRAW3D(item3D);
	}
	void MainWindow::saveRecentFilesList()
	{
		QFile saveFile(QStringLiteral("recentlyOpened.json"));

		if (!saveFile.open(QIODevice::WriteOnly)) {
			qWarning("Could not open \"recentlyOpened.json\".");
			return;
		}

		saveFile.write(m_importList.serialize().toJson());
	}
	void MainWindow::loadRecentFilesList()
	{
		m_importList.clear();

		if (!std::filesystem::exists(std::filesystem::path("recentlyOpened.json")))
		{
			return;
		}

		QFile loadFile(QStringLiteral("recentlyOpened.json"));

		if (!loadFile.open(QIODevice::ReadOnly)) {
			qWarning("Could not open \"recentlyOpened.json\".");
			return;
		}

		const QByteArray recentFiles = loadFile.readAll();
		const QJsonDocument loadDoc(QJsonDocument::fromJson(recentFiles));

		m_importList.deserialize(loadDoc);
	}
	void MainWindow::importRAW3D(const ImportItemRaw & item3D)
	{
		const VDTK::VolumeSize size = Helper::QVector3DToVolumeSize(item3D.getSize());
		const VDTK::VolumeSpacing spacing = Helper::QVector3DToVolumeSpacing(item3D.getSpacing());
		if (m_vdh.importRawFile(item3D.getFilePath(), item3D.getBitsPerVoxel(), size, spacing))
		{
			updateVolumeData();

			// add to recent files
			const ImportItem* item = &item3D;
			ImportItemListEntry* entry = new ImportItemListEntry(item, ImportType::RAW3D);
			m_importList.addImportItem(entry);
			saveRecentFilesList();
			refreshRecentFiles();
		}
		else
		{
			QMessageBox msgBox(QMessageBox::Warning, "Could not import 3D RAW", "Invalid import arguments.");
			msgBox.exec();
		}
	}
	void MainWindow::importRecentFile(std::size_t index)
	{
		const ImportItemListEntry* const entry = m_importList.getEntry(index);

		switch (entry->getType())
		{
		case VDS::ImportType::RAW3D:
		{
			const ImportItemRaw* const importItem = reinterpret_cast<const ImportItemRaw*>(entry->getItem());
			importRAW3D(*importItem);
			break;
		}
		case VDS::ImportType::BinarySlices:
		{
			Q_UNIMPLEMENTED();
			return;
			break;
		}
		case VDS::ImportType::BitmapSlices:
		{
			Q_UNIMPLEMENTED();
			return;
			break;
		}
		default:
		{
			Q_UNIMPLEMENTED();
			return;
			break;
		}
		}
	}

	void MainWindow::updateFrametime(float frameTime, float renderEverything, float volumeRendering)
	{
		ui.labelFPSValue->setText(QString::fromStdString(std::to_string(static_cast<uint16_t>(std::round(1000.0f / frameTime))) + " FPS"));
		ui.labelFrameTimeGlobalValue->setText(QString::fromStdString(std::to_string(renderEverything) + " ms"));
		ui.labelFrameTimeVolumeValue->setText(QString::fromStdString(std::to_string(volumeRendering) + " ms"));
	}

	void MainWindow::updateThresholdFromSlider(int threshold)
	{
		double thresholdValue = static_cast<double>(threshold) / 1000.0;

		ui.doubleSpinBoxThreshold->setValue(thresholdValue);
		ui.volumeViewWidget->setThreshold(thresholdValue);
	}

	void MainWindow::updateVolumeData()
	{
		const std::array<std::size_t, 3> size = {
			m_vdh.getVolumeData().getSize().getX(),
			m_vdh.getVolumeData().getSize().getY(),
			m_vdh.getVolumeData().getSize().getZ()
		};

		const std::array<float, 3> spacing = {
			m_vdh.getVolumeData().getSpacing().getX(),
			m_vdh.getVolumeData().getSpacing().getY(),
			m_vdh.getVolumeData().getSpacing().getZ()
		};

		ui.openGLWidgetHistogram->updateHistogramData(m_vdh.getHistogram());
		ui.volumeViewWidget->updateVolumeData(size, spacing, m_vdh.getVolumeData().getRawVolumeData());
	}

	void MainWindow::setupFileMenu()
	{
		m_menuFiles = new QMenu(ui.menuBar);
		m_menuFiles->setTitle(QString("File"));
		ui.menuBar->addMenu(m_menuFiles);

		m_actionImportRAW3D = new QAction(m_menuFiles);
		m_actionImportRAW3D->setText(QString("Import RAW 3D"));
		m_menuFiles->addAction(m_actionImportRAW3D);
		connect(m_actionImportRAW3D, &QAction::triggered, this, &MainWindow::openImportRawDialog);

		m_actionImportBinarySlices = new QAction(m_menuFiles);
		m_actionImportBinarySlices->setText(QString("Import Binary Slices"));
		m_menuFiles->addAction(m_actionImportBinarySlices);

		m_actionImportBitmapSeries = new QAction(m_menuFiles);
		m_actionImportBitmapSeries->setText(QString("Import Bitmap Series"));
		m_menuFiles->addAction(m_actionImportBitmapSeries);

		m_menuRecentFiles = new QMenu(m_menuFiles);
		m_menuRecentFiles->setTitle(QString("Recent Files"));
		m_menuFiles->addMenu(m_menuRecentFiles);

		m_menuFiles->addSeparator();

		m_actionExportRAW3D = new QAction(m_menuFiles);
		m_actionExportRAW3D->setText(QString("Export RAW 3D"));
		m_menuFiles->addAction(m_actionExportRAW3D);

		m_actionExportBitmapSeries = new QAction(m_menuFiles);
		m_actionExportBitmapSeries->setText(QString("Export Bitmap Series"));
		m_menuFiles->addAction(m_actionExportBitmapSeries);

		refreshRecentFiles();
	}

	void MainWindow::refreshRecentFiles()
	{
		for (auto action : m_menuRecentFiles->actions())
		{
			delete action;
		}

		loadRecentFilesList();

		for (int index = 0; index < m_importList.getSize(); index++)
		{
			QAction* action = new QAction(m_menuRecentFiles);
			action->setText(m_importList.getEntry(index)->getItem()->getFileName());
			m_menuRecentFiles->addAction(action);
			connect(action, &QAction::triggered, this, [this, index] { importRecentFile(index); });
		}
	}

}


