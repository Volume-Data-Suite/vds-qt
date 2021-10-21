
#include <QFileDialog>
#include <QMessageBox>
#include "import_binary_slices_dialog.h"

#include <limits>

namespace VDS {

DialogImportBinarySlices::DialogImportBinarySlices(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QString("Import Binary Slices"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupSectionPathToFile();
    setupSectionBitsPerVoxel();
    setupSectionEndianess();
    setupAxis();
    setupSectionSize();
    setupSectionSpacing();
    setupImportOrderPreview();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_groupPathToDirectory);
    m_vLayoutDialog->addWidget(m_groupBitsPerVoxel);
    m_vLayoutDialog->addWidget(m_groupEndianess);
    m_vLayoutDialog->addWidget(m_groupAxis);
    m_vLayoutDialog->addWidget(m_groupSize);
    m_vLayoutDialog->addWidget(m_groupSpacing);
    m_vLayoutDialog->addWidget(m_groupFileImportPreview);
    m_vLayoutDialog->addWidget(m_groupOKAndCancel);

    setLayout(m_vLayoutDialog);
    setMinimumSize(200, 200);

    m_numberOfSlices = 0;
}

const ImportItemBinarySlices DialogImportBinarySlices::getImportItem() const {
    const std::filesystem::path path =
        std::filesystem::path(m_textPathToDirectory->text().toStdString());
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

    VDTK::VolumeAxis axis;
    switch (m_comboBoxAxis->currentIndex()) {
    case 0:
        axis = VDTK::VolumeAxis::YZAxis;
        break;
    case 1:
        axis = VDTK::VolumeAxis::XZAxis;
        break;
    case 2:
    default:
        axis = VDTK::VolumeAxis::XYAxis;
        break;
    }

    return ImportItemBinarySlices(path, bitsPerVoxel, representedInLittleEndian, axis, size, spacing);
}
void DialogImportBinarySlices::onOKButtonClicked() {
    if (checkCurrentInput()) {
        this->accept();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Incomplete RAW 3D Import!",
                           "Some required fields are empty!");
        msgBox.exec();
    }
}
void DialogImportBinarySlices::onCancelButtonClicked() {
    this->reject();
}
void DialogImportBinarySlices::updateSizeDependingOnFileCount() {
    if (m_numberOfSlices == 0) {
        return;
    }
    
    m_textSizeX->setReadOnly(false);
    m_textSizeY->setReadOnly(false);
    m_textSizeZ->setReadOnly(false);

    switch (m_comboBoxAxis->currentIndex()) {
    case 2:
        m_textSizeZ->setText(QString::number(m_numberOfSlices));
        m_textSizeZ->setReadOnly(true);
        m_textSizeX->clear();
        m_textSizeY->clear();
        break;
    case 1:
        m_textSizeY->setText(QString::number(m_numberOfSlices));
        m_textSizeY->setReadOnly(true);
        m_textSizeX->clear();
        m_textSizeZ->clear();
        break;
    case 0:
    default:
        m_textSizeX->setText(QString::number(m_numberOfSlices));
        m_textSizeX->setReadOnly(true);
        m_textSizeY->clear();
        m_textSizeZ->clear();
        break;
    }
}

bool DialogImportBinarySlices::checkCurrentInput() {
    if (m_textPathToDirectory->text().isEmpty()) {
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
bool DialogImportBinarySlices::checkIsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}
void DialogImportBinarySlices::setupSectionPathToFile() {
    m_labelPathToDirectory = new QLabel;
    m_labelPathToDirectory->setText(QString("Path to Binary Slices directory:"));

    m_textPathToDirectory = new QLineEdit;
    m_textPathToDirectory->setReadOnly(true);
    m_textPathToDirectory->setMinimumWidth(350);
    m_textPathToDirectory->setPlaceholderText(QString("Select a directory"));

    m_buttonPathToDirectory = new QPushButton;
    m_buttonPathToDirectory->setText(QString("Open"));

    m_hLayoutPathToDirectory = new QHBoxLayout;
    m_hLayoutPathToDirectory->addWidget(m_textPathToDirectory);
    m_hLayoutPathToDirectory->addWidget(m_buttonPathToDirectory);

    m_vLayoutPathToDirectory = new QVBoxLayout;
    m_vLayoutPathToDirectory->addStretch();
    m_vLayoutPathToDirectory->addWidget(m_labelPathToDirectory);
    m_vLayoutPathToDirectory->addLayout(m_hLayoutPathToDirectory);
    m_vLayoutPathToDirectory->addStretch();

    m_groupPathToDirectory = new QGroupBox;
    m_groupPathToDirectory->setLayout(m_vLayoutPathToDirectory);

    connect(m_buttonPathToDirectory, &QPushButton::clicked, this, &DialogImportBinarySlices::selectDirectory);
}

void DialogImportBinarySlices::setupSectionBitsPerVoxel() {
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

void DialogImportBinarySlices::setupSectionEndianess() {
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

void DialogImportBinarySlices::setupSectionSize() {
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

void DialogImportBinarySlices::setupSectionSpacing() {
    m_labelSpacing = new QLabel;
    m_labelSpacing->setText(QString("Spacing in cm:"));

    // TODO: account float precision
    m_validatorSpacing = new QRegExpValidator(QRegExp("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"));

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

void DialogImportBinarySlices::setupSectionOKAndCancel() {
    m_buttonOK = new QPushButton;
    m_buttonOK->setText(QString("OK"));

    m_buttonCancel = new QPushButton;
    m_buttonCancel->setText(QString("Cancel"));

    m_hLayoutOKAndCancel = new QHBoxLayout;
    m_hLayoutOKAndCancel->addWidget(m_buttonOK);
    m_hLayoutOKAndCancel->addWidget(m_buttonCancel);

    m_groupOKAndCancel = new QGroupBox;
    m_groupOKAndCancel->setLayout(m_hLayoutOKAndCancel);

    connect(m_buttonOK, &QPushButton::clicked, this, &DialogImportBinarySlices::onOKButtonClicked);
    connect(m_buttonCancel, &QPushButton::clicked, this, &DialogImportBinarySlices::onCancelButtonClicked);
}

void DialogImportBinarySlices::setupImportOrderPreview() {
    m_labelFileImportPreview = new QLabel;
    m_labelFileImportPreview->setText(QString("File Import Order Preview:"));

    m_textFileImportPreview = new QTextEdit;
    m_textFileImportPreview->setPlaceholderText(QString("Files are imported by alphabetical order."));
    m_textFileImportPreview->setReadOnly(true);

    m_vLayoutFileImportPreview = new QVBoxLayout;
    m_vLayoutFileImportPreview->addWidget(m_labelFileImportPreview);
    m_vLayoutFileImportPreview->addWidget(m_textFileImportPreview);

    m_groupFileImportPreview = new QGroupBox;
    m_groupFileImportPreview->setLayout(m_vLayoutFileImportPreview);
}

void DialogImportBinarySlices::setupAxis() {
    m_labelAxis = new QLabel;
    m_labelAxis->setText(QString("Axis:"));

    m_comboBoxAxis = new QComboBox;
    m_comboBoxAxis->addItems({"Sliced along the YZ Axis", "Sliced along the XZ Axis", "Sliced along the XY Axis"});
    m_comboBoxAxis->setCurrentIndex(0);

    m_hLayoutAxis = new QHBoxLayout;
    m_hLayoutAxis->addWidget(m_labelAxis);
    m_hLayoutAxis->addWidget(m_comboBoxAxis);

    m_groupAxis = new QGroupBox;
    m_groupAxis->setLayout(m_hLayoutAxis);

    connect(m_comboBoxAxis, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogImportBinarySlices::updateSizeDependingOnFileCount);
}

void DialogImportBinarySlices::previewImportOrder() {
    const std::filesystem::path directory = m_textPathToDirectory->text().toStdString();
    QString fileList = "Files are imported by alphabetical order:\n\n";

    m_numberOfSlices = 0;
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directory)) {
        // skip subdirectories
        if (std::filesystem::is_regular_file(directoryEntry)) {
            const std::filesystem::path path = directoryEntry;
            fileList.append(QString::fromStdString(path.filename().string() + "\n"));
            m_numberOfSlices++;
        }
    }

    m_textFileImportPreview->setText(fileList);
}

void DialogImportBinarySlices::selectDirectory() {
    const QString path = QFileDialog::getExistingDirectory(nullptr, tr("Open Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (path.isEmpty()) {
        // Open file dialog got cancelled by user. Do not overwrite current text in case user open
        // file dialog a second time accidentally
        return;
    }
    m_textPathToDirectory->setText(path);
    previewImportOrder();
    updateSizeDependingOnFileCount();
}
} // namespace VDS
