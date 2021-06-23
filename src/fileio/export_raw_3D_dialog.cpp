
#include <QFileDialog>
#include <QMessageBox>
#include "export_raw_3D_dialog.h"

#include <limits>
#include <string>

namespace VDS {

DialogExportRAW3D::DialogExportRAW3D(const ValueWindow& valueWindow, const QVector3D& size,
                                     const QVector3D& spacing,
                                     QWidget* parent)
    : QDialog(parent), m_valueWindow(valueWindow), m_size(size), m_spacing(spacing) {
    setWindowTitle(QString("Export RAW 3D File"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupSectionPathToFile();
    setupSectionBitsPerVoxel();
    setupSectionMetaData();
    setupSectionValueWindow();
    setupSectionEndianess();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_groupPathToFile);
    m_vLayoutDialog->addWidget(m_metaData);
    m_vLayoutDialog->addWidget(m_valueWindowGroup);
    m_vLayoutDialog->addWidget(m_groupBitsPerVoxel);
    m_vLayoutDialog->addWidget(m_groupEndianess);
    m_vLayoutDialog->addWidget(m_groupOKAndCancel);

    setLayout(m_vLayoutDialog);
}

const ExportItemRaw DialogExportRAW3D::getExportItem() const {
    const std::filesystem::path path =
        std::filesystem::path(m_textPathToFile->text().toStdString());

    uint32_t bitsPerVoxel = 0;
    switch (m_comboBoxBitsPerVoxelOptions->currentIndex()) {
    case 0:
        bitsPerVoxel = 8;
        break;
    case 1:
    default:
        bitsPerVoxel = 16;
        break;
    }

    bool representedInLittleEndian = true;
    switch (m_comboBoxEndianessOptions->currentIndex()) {
    case 0:
        representedInLittleEndian = true;
        break;
    case 1:
    default:
        representedInLittleEndian = false;
        break;
    }

    const bool applyValueWindow = m_valueWindowGroup->isChecked();

    return ExportItemRaw(path, bitsPerVoxel, representedInLittleEndian, applyValueWindow);
}
void DialogExportRAW3D::deactiveSectionsIfNecessary(const int bitsPerVoxelIndex) {
    switch (bitsPerVoxelIndex) {
    case 0:
        if (checkIsBigEndian()) {
            m_comboBoxEndianessOptions->setCurrentIndex(0);
        } else {
            m_comboBoxEndianessOptions->setCurrentIndex(1);
        }
        m_groupEndianess->setDisabled(true);
        break;
    case 1:
    default:
        m_groupEndianess->setEnabled(true);
        break;
    }
}

void DialogExportRAW3D::onOKButtonClicked() {
    if (checkCurrentInput()) {
        this->accept();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Incomplete RAW 3D Export!",
                           "Some required fields are empty!");
        msgBox.exec();
    }
}
void DialogExportRAW3D::onCancelButtonClicked() {
    this->reject();
}
bool DialogExportRAW3D::checkCurrentInput() {
    if (m_textPathToFile->text().isEmpty()) {
        return false;
    }
    return true;
}
bool DialogExportRAW3D::checkIsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}
void DialogExportRAW3D::setupSectionPathToFile() {
    m_labelPathToFile = new QLabel;
    m_labelPathToFile->setText(QString("Path to RAW 3D file:"));

    m_textPathToFile = new QLineEdit;
    m_textPathToFile->setReadOnly(true);
    m_textPathToFile->setMinimumWidth(350);
    m_textPathToFile->setPlaceholderText(QString("Select path for .raw file"));

    m_buttonPathToFile = new QPushButton;
    m_buttonPathToFile->setText(QString("Select"));

    m_hLayoutPathToFile = new QHBoxLayout;
    m_hLayoutPathToFile->addWidget(m_textPathToFile);
    m_hLayoutPathToFile->addWidget(m_buttonPathToFile);

    m_vLayoutPathToFile = new QVBoxLayout;
    m_vLayoutPathToFile->addStretch();
    m_vLayoutPathToFile->addWidget(m_labelPathToFile);
    m_vLayoutPathToFile->addLayout(m_hLayoutPathToFile);
    m_vLayoutPathToFile->addStretch();

    m_groupPathToFile = new QGroupBox;
    m_groupPathToFile->setLayout(m_vLayoutPathToFile);

    connect(m_buttonPathToFile, &QPushButton::clicked, this, &DialogExportRAW3D::selectFile);
}

void DialogExportRAW3D::setupSectionMetaData() {
    // Size
    m_labelmetaDataSizeX = new QLabel;
    m_labelmetaDataSizeX->setText(QString("Size X: ") + QString::number(m_size.x()));
    m_labelmetaDataSizeY = new QLabel;
    m_labelmetaDataSizeY->setText(QString("Size Y: ") + QString::number(m_size.y()));
    m_labelmetaDataSizeZ = new QLabel;
    m_labelmetaDataSizeZ->setText(QString("Size Z: ") + QString::number(m_size.z()));

    m_vLayoutMetaDataSize = new QVBoxLayout;
    m_vLayoutMetaDataSize->addWidget(m_labelmetaDataSizeX);
    m_vLayoutMetaDataSize->addWidget(m_labelmetaDataSizeY);
    m_vLayoutMetaDataSize->addWidget(m_labelmetaDataSizeZ);

    m_metaDataSize = new QGroupBox;
    m_metaDataSize->setTitle(QString("Size:"));
    m_metaDataSize->setLayout(m_vLayoutMetaDataSize);

    // Spacing
    m_labelmetaDataSpacingX = new QLabel;
    m_labelmetaDataSpacingX->setText(QString("Spacing X: ") + QString::number(m_spacing.x()));
    m_labelmetaDataSpacingY = new QLabel;
    m_labelmetaDataSpacingY->setText(QString("Spacing Y: ") + QString::number(m_spacing.y()));
    m_labelmetaDataSpacingZ = new QLabel;
    m_labelmetaDataSpacingZ->setText(QString("Spacing Z: ") + QString::number(m_spacing.z()));

    m_vLayoutMetaDataSpacing = new QVBoxLayout;
    m_vLayoutMetaDataSpacing->addWidget(m_labelmetaDataSpacingX);
    m_vLayoutMetaDataSpacing->addWidget(m_labelmetaDataSpacingY);
    m_vLayoutMetaDataSpacing->addWidget(m_labelmetaDataSpacingZ);

    m_metaDataSpacing = new QGroupBox;
    m_metaDataSpacing->setTitle(QString("Spacing:"));
    m_metaDataSpacing->setLayout(m_vLayoutMetaDataSpacing);

    // Metadata
    m_hLayoutMetaData = new QHBoxLayout;
    m_hLayoutMetaData->addWidget(m_metaDataSize);
    m_hLayoutMetaData->addWidget(m_metaDataSpacing);

    m_metaData = new QGroupBox;
    m_metaData->setTitle(QString("Metadata:"));
    m_metaData->setLayout(m_hLayoutMetaData);
}

void DialogExportRAW3D::setupSectionValueWindow() {
    m_labelValueWindowInfo = new QLabel;
    m_labelValueWindowInfo->setText(QString(
        "If this section is selected, the current value window will be baked into the raw data."));

    m_verticalSpacer = new QSpacerItem(20, 15, QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_labelValueWindowFunction = new QLabel;
    m_labelValueWindowFunction->setText(QString("Function: ") + m_valueWindow.function);
    m_labelValueWindowWidth = new QLabel;
    m_labelValueWindowWidth->setText(QString("Window Width: ") +
                                     QString::number(m_valueWindow.width));
    m_labelValueWindowCenter = new QLabel;
    m_labelValueWindowCenter->setText(QString("Window Center: ") +
                                      QString::number(m_valueWindow.center));
    m_labelValueWindowOffset = new QLabel;
    m_labelValueWindowOffset->setText(QString("Window Offset: ") +
                                      QString::number(m_valueWindow.offset));
    
    m_vLayoutValueWindow = new QVBoxLayout;
    m_vLayoutValueWindow->addWidget(m_labelValueWindowInfo);
    m_vLayoutValueWindow->addItem(m_verticalSpacer);
    m_vLayoutValueWindow->addWidget(m_labelValueWindowFunction);
    m_vLayoutValueWindow->addWidget(m_labelValueWindowWidth);
    m_vLayoutValueWindow->addWidget(m_labelValueWindowCenter);
    m_vLayoutValueWindow->addWidget(m_labelValueWindowOffset);

    m_valueWindowGroup = new QGroupBox;
    m_valueWindowGroup->setTitle(QString("Bake in Value Window:"));
    m_valueWindowGroup->setCheckable(true);
    m_valueWindowGroup->setChecked(false);
    m_valueWindowGroup->setLayout(m_vLayoutValueWindow);
}

void DialogExportRAW3D::setupSectionBitsPerVoxel() {
    m_labelBitsPerVoxel = new QLabel;
    m_labelBitsPerVoxel->setText(QString("Bits per voxel:"));

    m_comboBoxBitsPerVoxelOptions = new QComboBox;
    m_comboBoxBitsPerVoxelOptions->addItems({"8", "16 (Recommended)"});
    m_comboBoxBitsPerVoxelOptions->setCurrentIndex(1);

    m_hLayoutBitsPerVoxel = new QHBoxLayout;
    m_hLayoutBitsPerVoxel->addWidget(m_labelBitsPerVoxel);
    m_hLayoutBitsPerVoxel->addWidget(m_comboBoxBitsPerVoxelOptions);

    m_groupBitsPerVoxel = new QGroupBox;
    m_groupBitsPerVoxel->setLayout(m_hLayoutBitsPerVoxel);

    connect(m_comboBoxBitsPerVoxelOptions,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogExportRAW3D::deactiveSectionsIfNecessary);
}

void DialogExportRAW3D::setupSectionEndianess() {
    m_labelEndianess = new QLabel;
    m_labelEndianess->setText(QString("Endianess:"));

    m_comboBoxEndianessOptions = new QComboBox;
    switch (checkIsBigEndian()) {
    case true:
        m_comboBoxEndianessOptions->addItems({"Big Endian (System Default)", "Little Endian"});
        m_comboBoxEndianessOptions->setCurrentIndex(0);
        break;
    case false:
    default:
        m_comboBoxEndianessOptions->addItems({"Big Endian", "Little Endian (System Default)"});
        m_comboBoxEndianessOptions->setCurrentIndex(1);
        break;
    }

    m_hLayoutEndianess = new QHBoxLayout;
    m_hLayoutEndianess->addWidget(m_labelEndianess);
    m_hLayoutEndianess->addWidget(m_comboBoxEndianessOptions);

    m_groupEndianess = new QGroupBox;
    m_groupEndianess->setLayout(m_hLayoutEndianess);
}

void DialogExportRAW3D::setupSectionOKAndCancel() {
    m_buttonOK = new QPushButton;
    m_buttonOK->setText(QString("OK"));

    m_buttonCancel = new QPushButton;
    m_buttonCancel->setText(QString("Cancel"));

    m_hLayoutOKAndCancel = new QHBoxLayout;
    m_hLayoutOKAndCancel->addWidget(m_buttonOK);
    m_hLayoutOKAndCancel->addWidget(m_buttonCancel);

    m_groupOKAndCancel = new QGroupBox;
    m_groupOKAndCancel->setLayout(m_hLayoutOKAndCancel);

    connect(m_buttonOK, &QPushButton::clicked, this, &DialogExportRAW3D::onOKButtonClicked);
    connect(m_buttonCancel, &QPushButton::clicked, this, &DialogExportRAW3D::onCancelButtonClicked);
}

void DialogExportRAW3D::selectFile() {
    const QString path =
        QFileDialog::getSaveFileName(nullptr, tr("Select File"), QDir::homePath(), tr("*.raw"));
    if (path.isEmpty()) {
        // Open file dialog got cancelled by user. Do not overwrite current text in case user open
        // file dialog a second time accidentally
        return;
    }
    m_textPathToFile->setText(path);
}
} // namespace VDS
