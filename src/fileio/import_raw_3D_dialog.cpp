
#include <QFileDialog>
#include <QMessageBox>
#include "import_raw_3D_dialog.h"

#include <limits>

namespace VDS {

DialogImportRAW3D::DialogImportRAW3D(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QString("Import RAW 3D File"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupSectionPathToFile();
    setupSectionBitsPerVoxel();
    setupSectionEndianess();
    setupSectionSize();
    setupSectionSpacing();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_groupPathToFile);
    m_vLayoutDialog->addWidget(m_groupBitsPerVoxel);
    m_vLayoutDialog->addWidget(m_groupEndianess);
    m_vLayoutDialog->addWidget(m_groupSize);
    m_vLayoutDialog->addWidget(m_groupSpacing);
    m_vLayoutDialog->addWidget(m_groupOKAndCancel);

    setLayout(m_vLayoutDialog);
    setMinimumSize(200, 200);
}

const ImportItemRaw DialogImportRAW3D::getImportItem() const {
    const std::filesystem::path path =
        std::filesystem::path(m_textPathToFile->text().toStdString());
    const QVector3D size(m_textSizeX->text().toFloat(), m_textSizeY->text().toFloat(),
                         m_textSizeZ->text().toFloat());
    const QVector3D spacing(m_textSpacingX->text().toFloat(), m_textSpacingY->text().toFloat(),
                            m_textSpacingZ->text().toFloat());

    uint32_t bitsPerVoxel = 0;
    switch (m_comboBoxBitsPerVoxelOptions->currentIndex()) {
    case 0:
        bitsPerVoxel = 8;
        break;
    case 1:
        bitsPerVoxel = 16;
        break;
    default:
        bitsPerVoxel = 0;
        break;
    }

    bool representedInLittleEndian = true;
    switch (m_comboBoxEndianessOptions->currentIndex()) {
    case 0:
        representedInLittleEndian = false;
        break;
    case 1:
    default:
        representedInLittleEndian = true;
        break;
    }

    return ImportItemRaw(path, bitsPerVoxel, representedInLittleEndian, size, spacing);
}
void DialogImportRAW3D::onOKButtonClicked() {
    if (checkCurrentInput()) {
        this->accept();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Incomplete RAW 3D Import!",
                           "Some required fields are empty!");
        msgBox.exec();
    }
}
void DialogImportRAW3D::onCancelButtonClicked() {
    this->reject();
}
bool DialogImportRAW3D::checkCurrentInput() {
    if (m_textPathToFile->text().isEmpty()) {
        return false;
    }
    if (m_textSizeX->text().isEmpty() || m_textSizeY->text().isEmpty() ||
        m_textSizeZ->text().isEmpty()) {
        return false;
    }
    if (m_textSpacingX->text().isEmpty() || m_textSpacingY->text().isEmpty() ||
        m_textSpacingZ->text().isEmpty()) {
        return false;
    }
    return true;
}
bool DialogImportRAW3D::checkIsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}
void DialogImportRAW3D::setupSectionPathToFile() {
    m_labelPathToFile = new QLabel;
    m_labelPathToFile->setText(QString("Path to RAW 3D file:"));

    m_textPathToFile = new QLineEdit;
    m_textPathToFile->setReadOnly(true);
    m_textPathToFile->setMinimumWidth(350);
    m_textPathToFile->setPlaceholderText(QString("Select a .raw file"));

    m_buttonPathToFile = new QPushButton;
    m_buttonPathToFile->setText(QString("Open"));

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

    connect(m_buttonPathToFile, &QPushButton::clicked, this, &DialogImportRAW3D::selectFile);
}

void DialogImportRAW3D::setupSectionBitsPerVoxel() {
    m_labelBitsPerVoxel = new QLabel;
    m_labelBitsPerVoxel->setText(QString("Bits per voxel:"));

    m_comboBoxBitsPerVoxelOptions = new QComboBox;
    m_comboBoxBitsPerVoxelOptions->addItems({"8", "16"});

    m_hLayoutBitsPerVoxel = new QHBoxLayout;
    m_hLayoutBitsPerVoxel->addWidget(m_labelBitsPerVoxel);
    m_hLayoutBitsPerVoxel->addWidget(m_comboBoxBitsPerVoxelOptions);

    m_groupBitsPerVoxel = new QGroupBox;
    m_groupBitsPerVoxel->setLayout(m_hLayoutBitsPerVoxel);
}

void DialogImportRAW3D::setupSectionEndianess() {
    m_labelEndianess = new QLabel;
    m_labelEndianess->setText(QString("Endianess:"));

    m_comboBoxEndianessOptions = new QComboBox;
    if (checkIsBigEndian()) {
        m_comboBoxEndianessOptions->addItems({"Big Endian (System Default)", "Little Endian"});
        m_comboBoxEndianessOptions->setCurrentIndex(0);
    } else {
        m_comboBoxEndianessOptions->addItems({"Big Endian", "Little Endian (System Default)"});
        m_comboBoxEndianessOptions->setCurrentIndex(1);
    }

    m_hLayoutEndianess = new QHBoxLayout;
    m_hLayoutEndianess->addWidget(m_labelEndianess);
    m_hLayoutEndianess->addWidget(m_comboBoxEndianessOptions);

    m_groupEndianess = new QGroupBox;
    m_groupEndianess->setLayout(m_hLayoutEndianess);
}

void DialogImportRAW3D::setupSectionSize() {
    m_labelSize = new QLabel;
    m_labelSize->setText(QString("Size in pixel:"));

    m_validatorSize = new QIntValidator(0, std::numeric_limits<int>::max(), this);

    m_textSizeX = new QLineEdit;
    m_textSizeX->setPlaceholderText(QString("X-Dimension"));
    m_textSizeX->setValidator(m_validatorSize);

    m_textSizeY = new QLineEdit;
    m_textSizeY->setPlaceholderText(QString("Y-Dimension"));
    m_textSizeY->setValidator(m_validatorSize);

    m_textSizeZ = new QLineEdit;
    m_textSizeZ->setPlaceholderText(QString("Z-Dimension"));
    m_textSizeZ->setValidator(m_validatorSize);

    m_hLayoutSize = new QHBoxLayout;
    m_hLayoutSize->addWidget(m_textSizeX);
    m_hLayoutSize->addWidget(m_textSizeY);
    m_hLayoutSize->addWidget(m_textSizeZ);

    m_vLayoutSize = new QVBoxLayout;
    m_vLayoutSize->addStretch();
    m_vLayoutSize->addWidget(m_labelSize);
    m_vLayoutSize->addLayout(m_hLayoutSize);
    m_vLayoutSize->addStretch();

    m_groupSize = new QGroupBox;
    m_groupSize->setLayout(m_vLayoutSize);
}

void DialogImportRAW3D::setupSectionSpacing() {
    m_labelSpacing = new QLabel;
    m_labelSpacing->setText(QString("Spacing in cm:"));

    // TODO: account float precision
    m_validatorSpacing = new QRegularExpressionValidator(QRegularExpression("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"));

    m_textSpacingX = new QLineEdit;
    m_textSpacingX->setPlaceholderText(QString("X-Dimension"));
    m_textSpacingX->setValidator(m_validatorSpacing);

    m_textSpacingY = new QLineEdit;
    m_textSpacingY->setPlaceholderText(QString("Y-Dimension"));
    m_textSpacingY->setValidator(m_validatorSpacing);

    m_textSpacingZ = new QLineEdit;
    m_textSpacingZ->setPlaceholderText(QString("Z-Dimension"));
    m_textSpacingZ->setValidator(m_validatorSpacing);

    m_hLayoutSpacing = new QHBoxLayout;
    m_hLayoutSpacing->addWidget(m_textSpacingX);
    m_hLayoutSpacing->addWidget(m_textSpacingY);
    m_hLayoutSpacing->addWidget(m_textSpacingZ);

    m_vLayoutSpacing = new QVBoxLayout;
    m_vLayoutSpacing->addStretch();
    m_vLayoutSpacing->addWidget(m_labelSpacing);
    m_vLayoutSpacing->addLayout(m_hLayoutSpacing);
    m_vLayoutSpacing->addStretch();

    m_groupSpacing = new QGroupBox;
    m_groupSpacing->setLayout(m_vLayoutSpacing);
}

void DialogImportRAW3D::setupSectionOKAndCancel() {
    m_buttonOK = new QPushButton;
    m_buttonOK->setText(QString("OK"));

    m_buttonCancel = new QPushButton;
    m_buttonCancel->setText(QString("Cancel"));

    m_hLayoutOKAndCancel = new QHBoxLayout;
    m_hLayoutOKAndCancel->addWidget(m_buttonOK);
    m_hLayoutOKAndCancel->addWidget(m_buttonCancel);

    m_groupOKAndCancel = new QGroupBox;
    m_groupOKAndCancel->setLayout(m_hLayoutOKAndCancel);

    connect(m_buttonOK, &QPushButton::clicked, this, &DialogImportRAW3D::onOKButtonClicked);
    connect(m_buttonCancel, &QPushButton::clicked, this, &DialogImportRAW3D::onCancelButtonClicked);
}

void DialogImportRAW3D::selectFile() {
    const QString path =
        QFileDialog::getOpenFileName(nullptr, tr("Select File"), QDir::homePath(), tr("*.raw"));
    if (path.isEmpty()) {
        // Open file dialog got cancelled by user. Do not overwrite current text in case user open
        // file dialog a second time accidentally
        return;
    }
    m_textPathToFile->setText(path);
}
} // namespace VDS
