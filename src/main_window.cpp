#include "main_window.h"

#include "fileio/import_binary_slices_dialog.h"
#include "fileio/import_raw_3D_dialog.h"
#include "fileio/export_raw_3D_dialog.h"
#include "fileio/export_image_series_dialog.h"
#include "tools/resize_volume_data.h"

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
    qRegisterMetaType<std::array<std::size_t, 3>>("std::array<std::size_t, 3>");
    qRegisterMetaType<std::array<float, 3>>("std::array<float, 3>");

    ui.setupUi(this);

    setWindowTitle(QString("Volume Data Suite"));

    setupViewMenu();
    setupFileMenu();
    setupToolsMenu();
    setupRendererView();
    setupShaderEditor();

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

    // connect volume data update
    connect(this, &MainWindow::updateVolumeView, ui.volumeViewWidget,
            &VolumeViewGL::updateVolumeData);

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

    // connect UI permisson updates
    connect(this, &MainWindow::updateUIPermissions, this, &MainWindow::setUIPermissions);

    // connect bounding box settings
    connect(ui.checkBoxRenderBoundingBox, &QCheckBox::stateChanged, ui.volumeViewWidget,
            &VolumeViewGL::setBoundingBoxRenderStatus);

    // connect error dialogs
    connect(this, &MainWindow::showErrorExportRaw, this, &MainWindow::errorRawExport);
    connect(this, &MainWindow::showErrorExportImagesSeries, this,
            &MainWindow::errorImageSeriesExport);
    connect(this, &MainWindow::showErrorImportRaw, this, &MainWindow::errorRawImport);
    connect(this, &MainWindow::showErrorImportBinarySlices, this,
            &MainWindow::errorBinarySlicesImport);

    // connect recent files
    connect(this, &MainWindow::updateRecentFiles, this, &MainWindow::refreshRecentFileList);

    // connect Shader Debug Editor
    connect(ui.volumeViewWidget, &VolumeViewGL::sendVertexShaderToUI, this,
            &MainWindow::setVertexDebugShaderEditor);
    connect(ui.volumeViewWidget, &VolumeViewGL::sendFragmentShaderToUI, this,
            &MainWindow::setFragmentDebugShaderEditor);
    connect(this, &MainWindow::updateVertexShaderFromEditor, ui.volumeViewWidget,
            &VolumeViewGL::recieveVertexShaderFromUI);
    connect(this, &MainWindow::updateFragmentShaderFromEditor, ui.volumeViewWidget,
            &VolumeViewGL::recieveFragmentShaderFromUI);
    connect(m_buttonApplyVertexShader, &QAbstractButton::clicked, this,
            &MainWindow::triggerManualVertexShaderUpdateFromEditor);
    connect(m_buttonApplyFragmentShader, &QAbstractButton::clicked, this,
            &MainWindow::triggerManualFragmentShaderUpdateFromEditor);

    // Disable all exports until a file is loaded
    m_actionExportRAW3D->setEnabled(false);
    m_actionExportBitmapSeries->setEnabled(false);
    m_actionResizeVolumeData->setEnabled(false);
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
        m_actionImportBinarySlices->setEnabled(true);
        m_menuRecentFiles->setEnabled(true);
        m_actionResizeVolumeData->setEnabled(true);
        ui.groupBoxApplyWindow->setEnabled(true);
        break;
    default:
        // disable write access UI elements
        m_actionImportRAW3D->setEnabled(false);
        m_actionImportBinarySlices->setEnabled(false);
        m_menuRecentFiles->setEnabled(false);
        m_actionResizeVolumeData->setEnabled(false);
        ui.groupBoxApplyWindow->setEnabled(false);
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

void MainWindow::openImportBinarySlicesDialog() {
    emit(updateUIPermissions(1, 1));
    DialogImportBinarySlices dialog;
    dialog.show();

    if (dialog.exec() != QDialog::Accepted) {
        // Raw Import got canceled by user
        emit(updateUIPermissions(-1, -1));
        return;
    }

    const ImportItemBinarySlices item3D = dialog.getImportItem();
    importBinarySlices(item3D);
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
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::currentThread()->setObjectName("Import Raw Thread");
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
            emit(updateRecentFiles());
        } else {
            emit(showErrorImportRaw());
        }
        emit(updateUIPermissions(-1, -1));

        return;
    });
}
void MainWindow::importBinarySlices(const ImportItemBinarySlices& item3D) {
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::currentThread()->setObjectName("Import Raw Thread");
        emit(updateUIPermissions(1, 1));

        const VDTK::VolumeSize size = Helper::QVector3DToVolumeSize(item3D.getSize());
        const VDTK::VolumeSpacing spacing = Helper::QVector3DToVolumeSpacing(item3D.getSpacing());

        if (m_vdh.importBinarySlices(item3D.getFilePath(), item3D.getBitsPerVoxel(), item3D.getAxis(), size,
                                     spacing)) {
            if (item3D.representedInLittleEndian() == checkIsBigEndian()) {
                m_vdh.convertEndianness();
            }

            updateVolumeData();

            // add to recent files
            const ImportItem* item = &item3D;
            ImportItemListEntry* entry = new ImportItemListEntry(item, ImportType::BinarySlices);
            m_importList.addImportItem(entry);
            saveRecentFilesList();
            emit(updateRecentFiles());
        } else {
            emit(showErrorImportBinarySlices());
        }
        emit(updateUIPermissions(-1, -1));

        return;
    });
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
        const ImportItemBinarySlices* const importItem =
            reinterpret_cast<const ImportItemBinarySlices*>(entry->getItem());
        importBinarySlices(*importItem);
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

    const QVector3D size(m_vdh.getVolumeData().getSize().getX(), m_vdh.getVolumeData().getSize().getY(),
                   m_vdh.getVolumeData().getSize().getZ());
    const QVector3D spacing(m_vdh.getVolumeData().getSpacing().getX(),
                      m_vdh.getVolumeData().getSpacing().getY(),
                      m_vdh.getVolumeData().getSpacing().getZ());

    const int32_t windowWidth = ui.spinBoxApplyWindowValueWindowWidth->value();
    const int32_t windowCenter = ui.spinBoxApplyWindowValueWindowCenter->value();
    const int32_t windowOffset = ui.spinBoxApplyWindowValueWindowOffset->value();
    const QString function = ui.comboBoxApplyWindowFunction->currentText();
    const ValueWindow valueWindow = ValueWindow(function, windowWidth, windowCenter, windowOffset);

    DialogExportRAW3D dialog(valueWindow, size, spacing);
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
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::currentThread()->setObjectName("Export Raw Thread");
        emit(updateUIPermissions(0, 1));
        auto vdhCopy = m_vdh;
        emit(updateUIPermissions(0, -1));

        const bool convertEndianness = item.representedInLittleEndian() != checkIsBigEndian();

        if (item.applyValueWindow()) {
            const int32_t windowWidth = ui.spinBoxApplyWindowValueWindowWidth->value();
            const int32_t windowCenter = ui.spinBoxApplyWindowValueWindowCenter->value();
            const int32_t windowOffset = ui.spinBoxApplyWindowValueWindowOffset->value();
            const VDTK::WindowingFunction function =
                VDTK::WindowingFunction(ui.comboBoxApplyWindowFunction->currentIndex());
            vdhCopy.applyWindow(function, windowCenter, windowWidth, windowOffset);
        }

        if (convertEndianness) {
            vdhCopy.convertEndianness();
        }

        bool success = vdhCopy.exportRawFile(item.getPath(), item.getBitsPerVoxel());

        if (!success) {
            emit(showErrorExportRaw());
        }

        return;
    });
}

void MainWindow::openExportImageSeriesDialog() {
    emit(updateUIPermissions(1, 1));

    const QVector3D size(m_vdh.getVolumeData().getSize().getX(),
                         m_vdh.getVolumeData().getSize().getY(),
                         m_vdh.getVolumeData().getSize().getZ());
    const QVector3D spacing(m_vdh.getVolumeData().getSpacing().getX(),
                            m_vdh.getVolumeData().getSpacing().getY(),
                            m_vdh.getVolumeData().getSpacing().getZ());

    const int32_t windowWidth = ui.spinBoxApplyWindowValueWindowWidth->value();
    const int32_t windowCenter = ui.spinBoxApplyWindowValueWindowCenter->value();
    const int32_t windowOffset = ui.spinBoxApplyWindowValueWindowOffset->value();
    const QString function = ui.comboBoxApplyWindowFunction->currentText();
    const ValueWindow valueWindow = ValueWindow(function, windowWidth, windowCenter, windowOffset);

    DialogExportImageSeries dialog(size, spacing);
    dialog.show();

    if (dialog.exec() != QDialog::Accepted) {
        // Raw Export got canceled by user
        emit(updateUIPermissions(-1, -1));
        return;
    }

    // Call Export
    const ExportItemImageSeries item = dialog.getExportItem();
    exportImageSeries(item);
    emit(updateUIPermissions(-1, -1));
}

void MainWindow::exportImageSeries(const ExportItemImageSeries& item) {
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::currentThread()->setObjectName("Export Images Series Thread");
        emit(updateUIPermissions(0, 1));
        auto vdhCopy = m_vdh;

        if (item.applyValueWindow()) {
            const int32_t windowWidth = ui.spinBoxApplyWindowValueWindowWidth->value();
            const int32_t windowCenter = ui.spinBoxApplyWindowValueWindowCenter->value();
            const int32_t windowOffset = ui.spinBoxApplyWindowValueWindowOffset->value();
            const VDTK::WindowingFunction function =
                VDTK::WindowingFunction(ui.comboBoxApplyWindowFunction->currentIndex());
            vdhCopy.applyWindow(function, windowCenter, windowWidth, windowOffset);
        }

        bool success = vdhCopy.exportToBitmapMonochrom(item.getPath());

        if (!success) {
            emit(showErrorExportImagesSeries());
        }

        emit(updateUIPermissions(0, -1));
    });
}

void MainWindow::openVolumeDataResizeDialog() {
    emit(updateUIPermissions(1, 1));

    const QVector3D size(m_vdh.getVolumeSize().getX(),
                         m_vdh.getVolumeSize().getY(),
                         m_vdh.getVolumeSize().getZ());
    const QVector3D spacing(m_vdh.getVolumeSpacing().getX(),
                            m_vdh.getVolumeSpacing().getY(),
                            m_vdh.getVolumeSpacing().getZ());

    DialogResizeVolumeData dialog(size, spacing, ui.volumeViewWidget->getTextureSizeMaximum());
    connect(&dialog, &DialogResizeVolumeData::requestVRAMinfoUpdate, ui.volumeViewWidget,
            &VolumeViewGL::recieveVRAMinfoUpdateRequest);
    connect(ui.volumeViewWidget, &VolumeViewGL::sendVRAMinfoUpdate, &dialog,
            &DialogResizeVolumeData::recieveVRAMinfoUpdate);
    dialog.show();

    if (dialog.exec() != QDialog::Accepted) {
        // Raw Export got canceled by user
        emit(updateUIPermissions(-1, -1));
        return;
    }

    // Call Export
    resizeVolumeData(dialog.getNewSize(), dialog.getInterploationMethod());

    emit(updateUIPermissions(-1, -1));
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

void MainWindow::resizeVolumeData(QVector3D newSize, int interpolationMethod) {
    QFuture<void> future = QtConcurrent::run([=]() {
        QThread::currentThread()->setObjectName("Resize Volume Data Thread");
        emit(updateUIPermissions(1, 1));

        VDTK::ScaleMode scaleMode;
        switch (interpolationMethod) {
        case 2:
            scaleMode = VDTK::ScaleMode::Cubic;
            break;
        case 1:
            scaleMode = VDTK::ScaleMode::Linear;
            break;
        case 0:
        default:
            scaleMode = VDTK::ScaleMode::NearestNeighbor;
            break;
        }

        VDTK::VolumeSize size(newSize.x(), newSize.y(), newSize.z());

        m_vdh.scaleToSize(scaleMode, size);

        updateVolumeData();

        emit(updateUIPermissions(-1, -1));

        return;
    });
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

void MainWindow::setVertexDebugShaderEditor(const QString& vertexShader) {
    m_vertexShaderEdit->setText(vertexShader);
}

void MainWindow::setFragmentDebugShaderEditor(const QString& fragmentShader) {
    m_fragmentShaderEdit->setText(fragmentShader);
}

void MainWindow::triggerManualVertexShaderUpdateFromEditor() {
    updateVertexShaderFromEditor(m_vertexShaderEdit->toPlainText());
}

void MainWindow::triggerManualFragmentShaderUpdateFromEditor() {
    updateFragmentShaderFromEditor(m_fragmentShaderEdit->toPlainText());
}

void MainWindow::errorRawExport() {
    QMessageBox msgBox(QMessageBox::Critical, "Could not export RAW file",
                       "Could not export RAW file.");
    msgBox.exec();
}

void MainWindow::errorImageSeriesExport() {
    QMessageBox msgBox(QMessageBox::Critical, "Could not export images series",
                       "Could not export images series.");
    msgBox.exec();
}

void MainWindow::errorRawImport() {
    QMessageBox msgBox(QMessageBox::Warning, "Could not import 3D RAW",
                       "Invalid import arguments.");
    msgBox.exec();
}

void MainWindow::errorBinarySlicesImport() {
    QMessageBox msgBox(QMessageBox::Warning, "Could not import Binary Slices",
                       "Invalid import arguments.");
    msgBox.exec();
}

void MainWindow::toggleSliceViewEnabled() {
    if (m_actionToggleSliceView->isChecked()) {
        ui.groupBoxSliceRenderX->setHidden(false);
        ui.groupBoxSliceRenderY->setHidden(false);
        ui.groupBoxSliceRenderZ->setHidden(false);
    } else {
        ui.groupBoxSliceRenderX->setHidden(true);
        ui.groupBoxSliceRenderY->setHidden(true);
        ui.groupBoxSliceRenderZ->setHidden(true);
    }
}

void MainWindow::toggleControllViewEnabled() {
    if (m_actionToggleControlView->isChecked()) {
        ui.tabSettings->setHidden(false);
    } else {
        ui.tabSettings->setHidden(true);
    }
}

void MainWindow::enableSliceRendererXMetaInfo() {    
    ui.groupBoxSliceRenderX->setStyleSheet(
        "QGroupBox { border: 2px solid red; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 1px solid red; background: red; } "
        "QSlider::handle:vertical { border: 1px solid red; background: red; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererX->setHidden(false);
}

void MainWindow::enableSliceRendererYMetaInfo() {    
    ui.groupBoxSliceRenderY->setStyleSheet(
        "QGroupBox { border: 2px solid green; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 1px solid green; background: green; } "
        "QSlider::handle:vertical { border: 1px solid green; background: green; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererY->setHidden(false);
}

void MainWindow::enableSliceRendererZMetaInfo() {    
    ui.groupBoxSliceRenderZ->setStyleSheet(
        "QGroupBox { border: 2px solid blue; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 1px solid blue; background: blue; } "
        "QSlider::handle:vertical { border: 1px solid blue; background: blue; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererZ->setHidden(false);
}

void MainWindow::disableSliceRendererXMetaInfo() {
    ui.groupBoxSliceRenderX->setStyleSheet(
        "QGroupBox { border: 2px solid red; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid red; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid red; background: transparent; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererX->setHidden(true);
}

void MainWindow::disableSliceRendererYMetaInfo() {
    ui.groupBoxSliceRenderY->setStyleSheet(
        "QGroupBox { border: 2px solid green; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid green; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid green; background: transparent; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererY->setHidden(true);
}

void MainWindow::disableSliceRendererZMetaInfo() {
    ui.groupBoxSliceRenderZ->setStyleSheet(
        "QGroupBox { border: 2px solid blue; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid blue; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid blue; background: transparent; } "
        "QSlider { background: transparent; }");

    m_labelSliceRendererZ->setHidden(true);
}

void MainWindow::updateVolumeData() {
    const std::array<std::size_t, 3> size = {m_vdh.getVolumeData().getSize().getX(),
                                             m_vdh.getVolumeData().getSize().getY(),
                                             m_vdh.getVolumeData().getSize().getZ()};

    const std::array<float, 3> spacing = {m_vdh.getVolumeData().getSpacing().getX(),
                                          m_vdh.getVolumeData().getSpacing().getY(),
                                          m_vdh.getVolumeData().getSpacing().getZ()};

    emit(updateVolumeView(size, spacing, m_vdh.getVolumeData().getRawVolumeData()));

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
    connect(m_actionImportBinarySlices, &QAction::triggered, this,
            &MainWindow::openImportBinarySlicesDialog);

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
    connect(m_actionExportBitmapSeries, &QAction::triggered, this,
            &MainWindow::openExportImageSeriesDialog);

    refreshRecentFileList();
}

void MainWindow::setupViewMenu() {
    m_menuView = new QMenu(ui.menuBar);
    m_menuView->setTitle(QString("View"));
    ui.menuBar->addMenu(m_menuView);

    m_actionResetView = new QAction(m_menuView);
    m_actionResetView->setText(QString("Reset View"));
    m_menuView->addAction(m_actionResetView);
    connect(m_actionResetView, &QAction::triggered, ui.volumeViewWidget,
            &VolumeViewGL::resetViewMatrixAndUpdate);

    m_actionToggleControlView = new QAction(m_menuView);
    m_actionToggleControlView->setText(QString("Show Controls"));
    m_actionToggleControlView->setCheckable(true);
    m_actionToggleControlView->setChecked(true);
    m_menuView->addAction(m_actionToggleControlView);
    connect(m_actionToggleControlView, &QAction::triggered, this,
            &MainWindow::toggleControllViewEnabled);

    m_actionToggleSliceView = new QAction(m_menuView);
    m_actionToggleSliceView->setText(QString("Show Slice View"));
    m_actionToggleSliceView->setCheckable(true);
    m_actionToggleSliceView->setChecked(true);
    m_menuView->addAction(m_actionToggleSliceView);
    connect(m_actionToggleSliceView, &QAction::triggered, this,
            &MainWindow::toggleSliceViewEnabled);
}

void MainWindow::setupToolsMenu() {
    m_menuTools = new QMenu(ui.menuBar);
    m_menuTools->setTitle(QString("Tools"));
    ui.menuBar->addMenu(m_menuTools);

    m_actionResizeVolumeData = new QAction(m_menuTools);
    m_actionResizeVolumeData->setText(QString("Resize Volume Data"));
    m_menuTools->addAction(m_actionResizeVolumeData);
    connect(m_actionResizeVolumeData, &QAction::triggered, this,
            &MainWindow::openVolumeDataResizeDialog);
}

void MainWindow::setupRendererView() {
    ui.groupBoxRenderer->setStyleSheet("QGroupBox { border: 0px solid white; margin-top: 0ex; } "
                                       "QGroupBox::title { padding: 0 0px; }");
    ui.groupBoxSliceRenderX->setStyleSheet(
        "QGroupBox { border: 2px solid red; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid red; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid red; background: transparent; } "
        "QSlider { background: transparent; }");
    ui.groupBoxSliceRenderY->setStyleSheet(
        "QGroupBox { border: 2px solid green; margin-top: 0ex; "
        "} QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid green; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid green; background: transparent; } "
        "QSlider { background: transparent; }");
    ui.groupBoxSliceRenderZ->setStyleSheet(
        "QGroupBox { border: 2px solid blue; margin-top: 0ex; } "
        "QGroupBox::title { padding: 0 0px; }"
        "QSlider::groove:vertical { border: 0px solid blue; background: transparent; } "
        "QSlider::handle:vertical { border: 0px solid blue; background: transparent; } "
        "QSlider { background: transparent; }");
    ui.groupBoxVolumeView->setStyleSheet("QGroupBox { border: 0px solid white; margin-top: 0ex; } "
                                         "QGroupBox::title { padding: 0 0px; }");


    m_labelSliceRendererX = new QLabel(ui.openGLWidgetSliceRenderX);
    m_labelSliceRendererX->setText("X-Axis");
    m_labelSliceRendererX->setAttribute(Qt::WA_TranslucentBackground);
    m_labelSliceRendererX->setStyleSheet("QLabel { color: red; }");
    m_labelSliceRendererX->move(3, -8);
    m_labelSliceRendererX->setHidden(true);

    m_labelSliceRendererY = new QLabel(ui.openGLWidgetSliceRenderY);
    m_labelSliceRendererY->setText("Y-Axis");
    m_labelSliceRendererY->setAttribute(Qt::WA_TranslucentBackground);
    m_labelSliceRendererY->setStyleSheet("QLabel { color: green; }");
    m_labelSliceRendererY->move(3, -8);
    m_labelSliceRendererY->setHidden(true);

    m_labelSliceRendererZ = new QLabel(ui.openGLWidgetSliceRenderZ);
    m_labelSliceRendererZ->setText("Z-Axis");
    m_labelSliceRendererZ->setAttribute(Qt::WA_TranslucentBackground);
    m_labelSliceRendererZ->setStyleSheet("QLabel { color: blue; }");
    m_labelSliceRendererZ->move(3, -8);
    m_labelSliceRendererZ->setHidden(true);
    
    m_sliderSliceRendererX = new QSlider();
    m_sliderSliceRendererXLayout = new QHBoxLayout();
    m_sliderSliceRendererXLayout->addStretch();
    m_sliderSliceRendererXLayout->addWidget(m_sliderSliceRendererX);
    ui.openGLWidgetSliceRenderX->setLayout(m_sliderSliceRendererXLayout);
        
    m_sliderSliceRendererY = new QSlider();
    m_sliderSliceRendererYLayout = new QHBoxLayout();
    m_sliderSliceRendererYLayout->addStretch();
    m_sliderSliceRendererYLayout->addWidget(m_sliderSliceRendererY);
    ui.openGLWidgetSliceRenderY->setLayout(m_sliderSliceRendererYLayout);

    m_sliderSliceRendererZ = new QSlider();
    m_sliderSliceRendererZLayout = new QHBoxLayout();
    m_sliderSliceRendererZLayout->addStretch();
    m_sliderSliceRendererZLayout->addWidget(m_sliderSliceRendererZ);
    ui.openGLWidgetSliceRenderZ->setLayout(m_sliderSliceRendererZLayout);

    connect(ui.openGLWidgetSliceRenderX, &SliceViewGL::enterEventSignaled, this,
            &MainWindow::enableSliceRendererXMetaInfo);
    connect(ui.openGLWidgetSliceRenderX, &SliceViewGL::leaveEventSignaled, this,
            &MainWindow::disableSliceRendererXMetaInfo);

    connect(ui.openGLWidgetSliceRenderY, &SliceViewGL::enterEventSignaled, this,
            &MainWindow::enableSliceRendererYMetaInfo);
    connect(ui.openGLWidgetSliceRenderY, &SliceViewGL::leaveEventSignaled, this,
            &MainWindow::disableSliceRendererYMetaInfo);

    connect(ui.openGLWidgetSliceRenderZ, &SliceViewGL::enterEventSignaled, this,
            &MainWindow::enableSliceRendererZMetaInfo);
    connect(ui.openGLWidgetSliceRenderZ, &SliceViewGL::leaveEventSignaled, this,
            &MainWindow::disableSliceRendererZMetaInfo);
}

void MainWindow::setupShaderEditor() {
    m_vertexShaderEdit = new QTextEdit();
    m_vertexShaderEdit->setPlaceholderText("Current Vertex Shader");
    m_vertexShaderEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    m_buttonApplyVertexShader = new QPushButton;
    m_buttonApplyVertexShader->setText(QString("Apply"));
    m_vertexShaderEditLayout = new QVBoxLayout();
    m_vertexShaderEditLayout->addWidget(m_vertexShaderEdit);
    m_vertexShaderEditLayout->addWidget(m_buttonApplyVertexShader);
    m_vertexShaderEditorSection = new ExpandableSectionWidget(QString("Vertex Shader"));
    m_vertexShaderEditorSection->setContentLayout(*m_vertexShaderEditLayout);

    m_fragmentShaderEdit = new QTextEdit();
    m_fragmentShaderEdit->setPlaceholderText("Current Fragment Shader");
    m_fragmentShaderEdit->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    m_buttonApplyFragmentShader = new QPushButton;
    m_buttonApplyFragmentShader->setText(QString("Apply"));
    m_fragmentShaderEditLayout = new QVBoxLayout();
    m_fragmentShaderEditLayout->addWidget(m_fragmentShaderEdit);
    m_fragmentShaderEditLayout->addWidget(m_buttonApplyFragmentShader);
    m_fragmentShaderEditorSection = new ExpandableSectionWidget(QString("Fragment Shader"));
    m_fragmentShaderEditorSection->setContentLayout(*m_fragmentShaderEditLayout);

    m_shaderEditorInfo = new QLabel();
    m_shaderEditorInfo->setWordWrap(true);
    m_shaderEditorInfo->setText(QString(
        "VDS generates its shaders dynamically depending on the different setting applied. "
        "In case you change things like the ray casting method, value "
        "windows and so on. a complete new shader is generated. This will overwrite all "
        "manual changes of the shader code within this debug editor.\nPlease be aware that this "
        "could break the visual output until you switch back to a ray casting method preset."));

    m_groupBoxShaderEditorLayout = new QVBoxLayout;
    m_groupBoxShaderEditorLayout->setContentsMargins(9, 17 , 9 , 9);
    m_groupBoxShaderEditorLayout->addWidget(m_shaderEditorInfo);
    m_groupBoxShaderEditorLayout->addWidget(m_vertexShaderEditorSection);
    m_groupBoxShaderEditorLayout->addWidget(m_fragmentShaderEditorSection);

    // remove the existing layout from the group box
    qDeleteAll(ui.groupBoxShaderEditor->children());
    ui.groupBoxShaderEditor->setLayout(m_groupBoxShaderEditorLayout);        
}

void MainWindow::refreshRecentFileList() {
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
