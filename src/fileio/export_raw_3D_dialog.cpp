
#include <QFileDialog>
#include <QMessageBox>
#include "export_raw_3D_dialog.h"

#include <limits>

namespace VDS {

DialogExportRAW3D::DialogExportRAW3D(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QString("Export RAW 3D File"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupSectionPathToFile();
    setupSectionBitsPerVoxel();
    setupSectionEndianess();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_groupPathToFile);
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

    return ExportItemRaw(path, bitsPerVoxel, representedInLittleEndian);
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

void DialogExportRAW3D::setupSectionBitsPerVoxel() {
    m_labelBitsPerVoxel = new QLabel;
    m_labelBitsPerVoxel->setText(QString("Bits per voxel:"));

    m_comboBoxBitsPerVoxelOptions = new QComboBox;
    m_comboBoxBitsPerVoxelOptions->addItems({"8", "16"});
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
    m_comboBoxEndianessOptions->addItems({"Big Endian", "Little Endian"});
    switch (checkIsBigEndian()) {
    case true:
        m_comboBoxEndianessOptions->setCurrentIndex(0);
        break;
    case false:
    default:
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
