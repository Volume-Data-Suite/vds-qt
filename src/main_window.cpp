#include "main_window.h"

#include "fileio/import_raw_3D_dialog.h"
#include "fileio/export_raw_3D_dialog.h"

#include "common/vdtk_helper_functions.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGroupBox>
#include <QJsonDocument>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFuture>

namespace VDS {
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    // Register meta types so concurrent threads can pass these in signals
    qRegisterMetaType<std::vector<uint16_t>>("std::vector<uint16_t>");

    ui.setupUi(this);

    setWindowTitle(QString("Volume Data Suite"));

    setupFileMenu();

    // as long as its not functional
    ui.groupBoxSliceView->hide();

    // connect debug infos
    connect(ui.volumeViewWidget, &VolumeViewGL::updateFrametime, this,
            &MainWindow::updateFrametime);
    connect(ui.checkBoxRenderLoop, &QCheckBox::stateChanged, ui.volumeViewWidget,
            &VolumeViewGL::setRenderLoop);

    // connect sample step length
    connect(ui.doubleSpinBoxSampleRate,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            ui.volumeViewWidget, &VolumeViewGL::setSampleStepLength);
    connect(ui.comboBoxSampleRateRecommended,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            ui.volumeViewWidget, &VolumeViewGL::setRecommendedSampleStepLength);
    connect(ui.volumeViewWidget, &VolumeViewGL::updateSampleStepLength, ui.doubleSpinBoxSampleRate,
            &QDoubleSpinBox::setValue);

    // connect threshold
    connect(ui.horizontalSliderThreshold, &QSlider::valueChanged, this,
            &MainWindow::updateThresholdFromSlider);

    // connect raycast method
    connect(ui.comboBoxShaderMethod,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            ui.volumeViewWidget, &VolumeViewGL::setRaycastMethod);

    // connect value window
    connect(ui.groupBoxApplyWindow, &QGroupBox::toggled, ui.volumeViewWidget,
            &VolumeViewGL::applyValueWindow);
    connect(ui.spinBoxApplyWindowValueWindowWidth,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.volumeViewWidget,
            &VolumeViewGL::updateValueWindowWidth);
    connect(ui.spinBoxApplyWindowValueWindowCenter,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.volumeViewWidget,
            &VolumeViewGL::updateValueWindowCenter);
    connect(ui.spinBoxApplyWindowValueWindowOffset,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui.volumeViewWidget,
            &VolumeViewGL::updateValueWindowOffset);
    connect(ui.comboBoxApplyWindowPresets, &QComboBox::currentTextChanged, this,
            &MainWindow::setValueWindowPreset);
    connect(ui.comboBoxApplyWindowFunction,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            ui.volumeViewWidget, &VolumeViewGL::setValueWindowMethod);

    // connect histogram update
    connect(ui.groupBoxApplyWindow, &QGroupBox::toggled, this, &MainWindow::computeHistogram);
    connect(ui.spinBoxApplyWindowValueWindowWidth,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MainWindow::computeHistogram);
    connect(ui.spinBoxApplyWindowValueWindowCenter,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MainWindow::computeHistogram);
    connect(ui.spinBoxApplyWindowValueWindowOffset,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MainWindow::computeHistogram);
    connect(ui.comboBoxApplyWindowFunction,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MainWindow::computeHistogram);
    connect(this, &MainWindow::updateHistogram, ui.openGLWidgetHistogram, &HistogramViewGL::updateHistogramData);
    connect(this, &MainWindow::updateUIPermissions, this, &MainWindow::setUIPermissions);

    // connect bounding box settings
    connect(ui.checkBoxRenderBoundingBox, &QCheckBox::stateChanged, ui.volumeViewWidget,
            &VolumeViewGL::setBoundingBoxRenderStatus);
}

void MainWindow::setUIPermissions(int read, int write) {
    readBlockCount += read;
    writeBlockCount += write;

    switch (readBlockCount) {
    case 0:
        // enable read only UI elements
        m_actionExportRAW3D->setEnabled(true);
        m_actionExportBitmapSeries->setEnabled(true);
        break;
    default:
        // disable read only UI elements
        m_actionExportRAW3D->setEnabled(false);
        m_actionExportBitmapSeries->setEnabled(false);
        break;
    }

    switch (writeBlockCount) {
    case 0:
        // enable write access UI elements
        m_actionImportRAW3D->setEnabled(true);
        m_actionImportBitmapSeries->setEnabled(true);
        m_actionImportBinarySlices->setEnabled(true);
        break;
    default:
        // disable write access UI elements
        m_actionImportRAW3D->setEnabled(false);
        m_actionImportBitmapSeries->setEnabled(false);
        m_actionImportBinarySlices->setEnabled(false);
        break;
    }
}


void MainWindow::openImportRawDialog() {
    emit(updateUIPermissions(1, 1));
    DialogImportRAW3D dialog;
    dialog.show();

    if (dialog.exec() != QDialog::Accepted) {
        // Raw Import got canceled by user
        emit(updateUIPermissions(-1, -1));
        return;
    }

    const ImportItemRaw item3D = dialog.getImportItem();
    importRAW3D(item3D);
    emit(updateUIPermissions(-1, -1));
}
void MainWindow::saveRecentFilesList() {
    QFile saveFile(QStringLiteral("recentlyOpened.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Could not open \"recentlyOpened.json\".");
        return;
    }

    saveFile.write(m_importList.serialize().toJson());
}
void MainWindow::loadRecentFilesList() {
    m_importList.clear();

    if (!std::filesystem::exists(std::filesystem::path("recentlyOpened.json"))) {
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
void MainWindow::importRAW3D(const ImportItemRaw& item3D) {
    emit(updateUIPermissions(1, 1));

    const VDTK::VolumeSize size = Helper::QVector3DToVolumeSize(item3D.getSize());
    const VDTK::VolumeSpacing spacing = Helper::QVector3DToVolumeSpacing(item3D.getSpacing());
    if (m_vdh.importRawFile(item3D.getFilePath(), item3D.getBitsPerVoxel(), size, spacing)) {
        if (item3D.representedInLittleEndian() == checkIsBigEndian()) {
            m_vdh.convertEndianness();
        }

        updateVolumeData();

        // add to recent files
        const ImportItem* item = &item3D;
        ImportItemListEntry* entry = new ImportItemListEntry(item, ImportType::RAW3D);
        m_importList.addImportItem(entry);
        saveRecentFilesList();
        refreshRecentFiles();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Could not import 3D RAW",
                           "Invalid import arguments.");
        msgBox.exec();
    }
    emit(updateUIPermissions(-1, -1));
}
void MainWindow::importRecentFile(std::size_t index) {
    const ImportItemListEntry* const entry = m_importList.getEntry(index);

    switch (entry->getType()) {
    case VDS::ImportType::RAW3D: {
        const ImportItemRaw* const importItem =
            reinterpret_cast<const ImportItemRaw*>(entry->getItem());
        importRAW3D(*importItem);
        break;
    }
    case VDS::ImportType::BinarySlices: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    case VDS::ImportType::BitmapSlices: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    default: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    }
}

void MainWindow::openExportRawDialog() {
    emit(updateUIPermissions(1, 1));
    DialogExportRAW3D dialog;
    dialog.show();

    if (dialog.exec() != QDialog::Accepted) {
        // Raw Export got canceled by user
        emit(updateUIPermissions(-1, -1));
        return;
    }

    // Call Export
    const ExportItemRaw item3D = dialog.getExportItem();
    exportRAW3D(item3D);
    emit(updateUIPermissions(-1, -1));
}

void MainWindow::exportRAW3D(const ExportItemRaw& item) {
    emit(updateUIPermissions(0, 1));

    if (item.representedInLittleEndian() != checkIsBigEndian()) {
        m_vdh.convertEndianness();
    }    

    if (!m_vdh.exportRawFile(item.getPath(), item.getBitsPerVoxel())) {
        QMessageBox msgBox(QMessageBox::Critical, "Could not export RAW file",
                           "Could not export RAW file.");
        msgBox.exec();
    } 

    emit(updateUIPermissions(0, -1));
}

void MainWindow::updateFrametime(float frameTime, float renderEverything, float volumeRendering) {
    ui.labelFPSValue->setText(QString::fromStdString(
        std::to_string(static_cast<uint16_t>(std::round(1000.0f / frameTime))) + " FPS"));
    ui.labelFrameTimeGlobalValue->setText(
        QString::fromStdString(std::to_string(renderEverything) + " ms"));
    ui.labelFrameTimeVolumeValue->setText(
        QString::fromStdString(std::to_string(volumeRendering) + " ms"));
}

void MainWindow::updateThresholdFromSlider(int threshold) {
    double thresholdValue = static_cast<double>(threshold) / 1000.0;

    ui.doubleSpinBoxThreshold->setValue(thresholdValue);
    ui.volumeViewWidget->setThreshold(thresholdValue);
}

void MainWindow::computeHistogram() {
    QFuture<void> future = QtConcurrent::run(
        [&]() {
            QThread::currentThread()->setObjectName("Compute Histogram Thread");
            emit(updateUIPermissions(0, -1));

            const bool windowingEnabled = ui.groupBoxApplyWindow->isChecked();

            bool ignoreBorders = windowingEnabled;

            const int32_t windowWidth = ui.spinBoxApplyWindowValueWindowWidth->value();
            const int32_t windowCenter = ui.spinBoxApplyWindowValueWindowCenter->value();
            const int32_t windowOffset = ui.spinBoxApplyWindowValueWindowOffset->value();
            const VDTK::WindowingFunction function =
                VDTK::WindowingFunction(ui.comboBoxApplyWindowFunction->currentIndex());

            std::vector<uint16_t> histogram{};

            if (windowingEnabled) {
                histogram = m_vdh.getHistogramWidthWindowing(function, windowCenter, windowWidth,
                                                             windowOffset);
            } else {
                histogram = m_vdh.getHistogram();
            }

            emit(updateHistogram(histogram, ignoreBorders));
            emit(updateUIPermissions(0, 1));

            return;
        });
}

void MainWindow::setValueWindowPreset(const QString& preset) {
    if (preset == "Default: No Change") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(UINT16_MAX);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(UINT16_MAX / 2);
    } else if (preset == "Head & Neck: Brain") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(80);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(40);
    } else if (preset == "Head & Neck: Subdural") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(300);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(100);
    } else if (preset == "Head & Neck: Stroke") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(8);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(32);
    } else if (preset == "Head & Neck: Temporal Bones") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(2800);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(600);
    } else if (preset == "Head & Neck: Soft Tissues") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(400);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(60);
    } else if (preset == "Chest: Lungs") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(1500);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(-600);
    } else if (preset == "Chest: Mediastinum") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(350);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(50);
    } else if (preset == "Abdomen: Soft Tissues") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(400);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(50);
    } else if (preset == "Abdomen: Liver") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(150);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(30);
    } else if (preset == "Spine: Soft Tissues") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(250);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(50);
    } else if (preset == "Spine: Bones") {
        ui.spinBoxApplyWindowValueWindowWidth->setValue(1800);
        ui.spinBoxApplyWindowValueWindowCenter->setValue(400);
    }
}

void MainWindow::updateVolumeData() {
    const std::array<std::size_t, 3> size = {m_vdh.getVolumeData().getSize().getX(),
                                             m_vdh.getVolumeData().getSize().getY(),
                                             m_vdh.getVolumeData().getSize().getZ()};

    const std::array<float, 3> spacing = {m_vdh.getVolumeData().getSpacing().getX(),
                                          m_vdh.getVolumeData().getSpacing().getY(),
                                          m_vdh.getVolumeData().getSpacing().getZ()};

    ui.volumeViewWidget->updateVolumeData(size, spacing, m_vdh.getVolumeData().getRawVolumeData());

    computeHistogram();
}

void MainWindow::setupFileMenu() {
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
    connect(m_actionExportRAW3D, &QAction::triggered, this, &MainWindow::openExportRawDialog);

    m_actionExportBitmapSeries = new QAction(m_menuFiles);
    m_actionExportBitmapSeries->setText(QString("Export Bitmap Series"));
    m_menuFiles->addAction(m_actionExportBitmapSeries);

    refreshRecentFiles();
}

void MainWindow::refreshRecentFiles() {
    for (auto action : m_menuRecentFiles->actions()) {
        delete action;
    }

    loadRecentFilesList();

    for (int index = 0; index < m_importList.getSize(); index++) {
        QAction* action = new QAction(m_menuRecentFiles);
        action->setText(m_importList.getEntry(index)->getItem()->getFileName());
        m_menuRecentFiles->addAction(action);
        connect(action, &QAction::triggered, this, [this, index] { importRecentFile(index); });
    }
}

bool MainWindow::checkIsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

} // namespace VDS
