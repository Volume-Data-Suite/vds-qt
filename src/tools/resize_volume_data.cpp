
#include <QFileDialog>
#include <QMessageBox>
#include "resize_volume_data.h"

#include <limits>
#include <string>

#include <cmath>

namespace VDS {

DialogResizeVolumeData::DialogResizeVolumeData(const QVector3D& size, const QVector3D& spacing,
                                               const int textureSizeMax,
                                               QWidget* parent)
    : QDialog(parent), m_sizeOriginal(size), m_spacingOriginal(spacing) {
    setWindowTitle(QString("Resize Volume Data"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // compute texture size
    computeTextureSizeOriginal();
    m_textureSizeNew = m_textureSizeOriginal;
    m_textureSizeDriverMax = textureSizeMax;

    setupSectionmetaDataOriginal();
    setupSectionMetaDataNew();
    setupSectionInterpolationMethod();
    setupSectionTextureSize();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_metaDataOriginal);
    m_vLayoutDialog->addWidget(m_metaDataNew);
    m_vLayoutDialog->addWidget(m_groupInterpolationMethod);
    m_vLayoutDialog->addWidget(m_textureSize);
    m_vLayoutDialog->addWidget(m_groupOKAndCancel);

    setLayout(m_vLayoutDialog);

    // surpress setGeometry warning. Probably related to some weird stylesheet parsing.
    setMinimumSize(443, 563);

    setupVRAMTimer();
}

void DialogResizeVolumeData::onOKButtonClicked() {
    if (checkCurrentInput()) {
        this->accept();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Volume Data Resize not possible!",
                           "Invalid Size.");
        msgBox.exec();
    }
}
QVector3D DialogResizeVolumeData::getNewSize() const {
    return QVector3D(m_lineEditMetaDataNewSizeX->text().toFloat(),
                     m_lineEditMetaDataNewSizeY->text().toFloat(),
                     m_lineEditMetaDataNewSizeZ->text().toFloat());
}
int DialogResizeVolumeData::getInterploationMethod() const {
    return m_comboBoxInterpolationMethod->currentIndex();
}
void DialogResizeVolumeData::onCancelButtonClicked() {
    this->reject();
}
void DialogResizeVolumeData::computeTextureSizeOriginal() {
    m_textureSizeOriginal = static_cast<float>(sizeof(std::uint16_t)) * m_sizeOriginal.x() *
                            m_sizeOriginal.y() * m_sizeOriginal.z() /
                            static_cast<float>(std::pow(1024, 2));
}
void DialogResizeVolumeData::computeTextureSizeNew() {
    m_textureSizeNew =
        static_cast<float>(sizeof(std::uint16_t)) * m_lineEditMetaDataNewSizeX->text().toFloat() *
        m_lineEditMetaDataNewSizeY->text().toFloat() *
        m_lineEditMetaDataNewSizeZ->text().toFloat() / static_cast<float>(std::pow(1024, 2));

    m_labelTextureSizeNew->setText(QString("New: ") + QString::number(m_textureSizeNew) + " MB");
}
void DialogResizeVolumeData::updateSpacingX() {
    const float newSizeX = m_lineEditMetaDataNewSizeX->text().toFloat();
    const float scaleFactor = m_sizeOriginal.x() / newSizeX;

    m_lineEditMetaDataNewSpacingX->setText(QString::number(m_spacingOriginal.x() * scaleFactor));

    computeTextureSizeNew();
}
void DialogResizeVolumeData::updateSpacingY() {
    const float newSizeY = m_lineEditMetaDataNewSizeY->text().toFloat();
    const float scaleFactor = m_sizeOriginal.y() / newSizeY;

    m_lineEditMetaDataNewSpacingY->setText(QString::number(m_spacingOriginal.y() * scaleFactor));

    computeTextureSizeNew();
}
void DialogResizeVolumeData::updateSpacingZ() {
    const float newSizeZ = m_lineEditMetaDataNewSizeZ->text().toFloat();
    const float scaleFactor = m_sizeOriginal.z() / newSizeZ;

    m_lineEditMetaDataNewSpacingZ->setText(QString::number(m_spacingOriginal.z() * scaleFactor));

    computeTextureSizeNew();
}
void DialogResizeVolumeData::recieveVRAMinfoUpdate(bool success, int dedicatedMemory,
                                                   int totalAvailableMemory,
                                                   int availableDedicatedMemory, int envictionCount,
                                                   int envictedMemory) {
    if (success) {
        m_labelTotalGPUVRAM->setText(QString("GPU Total VRAM: ") +
                                     QString::number(dedicatedMemory / 1024.0f) + " MB");
        m_availableGPUVRAM = availableDedicatedMemory / 1024.0f;
        m_labelAvailableGPUVRAM->setText(QString("GPU Available VRAM: ") +
                                         QString::number(m_availableGPUVRAM) + " MB");
    }
    updateVRAMLabels();
}
void DialogResizeVolumeData::updateVRAMLabels() {
    if (m_availableGPUVRAM * 0.8f > m_availableGPUVRAM - m_textureSizeNew &&
        m_availableGPUVRAM > m_textureSizeNew) {
        m_labelTextureSizeNew->setStyleSheet("QLabel { color : orange; }");
        m_labelAvailableGPUVRAM->setStyleSheet("QLabel { color : orange; }");
    } else if (m_availableGPUVRAM > m_textureSizeNew) {
        m_labelTextureSizeNew->setStyleSheet("QLabel { color : #80E015; }");
        m_labelAvailableGPUVRAM->setStyleSheet("QLabel { color : #80E015; }");
    } else if (m_availableGPUVRAM < m_textureSizeNew) {
        m_labelTextureSizeNew->setStyleSheet("QLabel { color : #FF483F; }");
        m_labelAvailableGPUVRAM->setStyleSheet("QLabel { color : #FF483F; }");
    }
    if (m_textureSizeNew <= 0.0f) {
        m_labelTextureSizeNew->setStyleSheet("QLabel { color : #FF483F; }");
    }
}
bool DialogResizeVolumeData::checkCurrentInput() {
    if (m_lineEditMetaDataNewSizeX->text().isEmpty() ||
        m_lineEditMetaDataNewSizeY->text().isEmpty() ||
        m_lineEditMetaDataNewSizeZ->text().isEmpty()) {
        return false;
    }
    if (m_lineEditMetaDataNewSpacingX->text().isEmpty() ||
        m_lineEditMetaDataNewSpacingY->text().isEmpty() ||
        m_lineEditMetaDataNewSpacingZ->text().isEmpty()) {
        return false;
    }
    return true;
}

void DialogResizeVolumeData::setupSectionmetaDataOriginal() {
    // Size
    m_labelMetaDataOriginalSizeX = new QLabel;
    m_labelMetaDataOriginalSizeX->setText(QString("Size X: ") +
                                          QString::number(m_sizeOriginal.x()));
    m_labelMetaDataOriginalSizeY = new QLabel;
    m_labelMetaDataOriginalSizeY->setText(QString("Size Y: ") +
                                          QString::number(m_sizeOriginal.y()));
    m_labelMetaDataOriginalSizeZ = new QLabel;
    m_labelMetaDataOriginalSizeZ->setText(QString("Size Z: ") +
                                          QString::number(m_sizeOriginal.z()));

    m_vLayoutmetaDataOriginalSize = new QVBoxLayout;
    m_vLayoutmetaDataOriginalSize->addWidget(m_labelMetaDataOriginalSizeX);
    m_vLayoutmetaDataOriginalSize->addWidget(m_labelMetaDataOriginalSizeY);
    m_vLayoutmetaDataOriginalSize->addWidget(m_labelMetaDataOriginalSizeZ);

    m_metaDataOriginalSize = new QGroupBox;
    m_metaDataOriginalSize->setTitle(QString("Size in pixel:"));
    m_metaDataOriginalSize->setLayout(m_vLayoutmetaDataOriginalSize);

    // Spacing
    m_labelMetaDataOriginalSpacingX = new QLabel;
    m_labelMetaDataOriginalSpacingX->setText(
        QString("Spacing X: ") + QString::number(m_spacingOriginal.x()) + QString(" cm"));
    m_labelMetaDataOriginalSpacingY = new QLabel;
    m_labelMetaDataOriginalSpacingY->setText(
        QString("Spacing Y: ") + QString::number(m_spacingOriginal.y()) + QString(" cm"));
    m_labelMetaDataOriginalSpacingZ = new QLabel;
    m_labelMetaDataOriginalSpacingZ->setText(
        QString("Spacing Z: ") + QString::number(m_spacingOriginal.z()) + QString(" cm"));

    m_vLayoutmetaDataOriginalSpacing = new QVBoxLayout;
    m_vLayoutmetaDataOriginalSpacing->addWidget(m_labelMetaDataOriginalSpacingX);
    m_vLayoutmetaDataOriginalSpacing->addWidget(m_labelMetaDataOriginalSpacingY);
    m_vLayoutmetaDataOriginalSpacing->addWidget(m_labelMetaDataOriginalSpacingZ);

    m_metaDataOriginalSpacing = new QGroupBox;
    m_metaDataOriginalSpacing->setTitle(QString("Spacing in cm:"));
    m_metaDataOriginalSpacing->setLayout(m_vLayoutmetaDataOriginalSpacing);

    // metaDataOriginal
    m_hLayoutmetaDataOriginal = new QHBoxLayout;
    m_hLayoutmetaDataOriginal->addWidget(m_metaDataOriginalSize);
    m_hLayoutmetaDataOriginal->addWidget(m_metaDataOriginalSpacing);

    m_metaDataOriginal = new QGroupBox;
    m_metaDataOriginal->setTitle(QString("Original size and spacing:"));
    m_metaDataOriginal->setLayout(m_hLayoutmetaDataOriginal);
}

void DialogResizeVolumeData::setupSectionMetaDataNew() {
    // Validators
    m_validatorSize = new QIntValidator(0, std::numeric_limits<int>::max(), this);
    m_validatorSpacing = new QRegExpValidator(QRegExp("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"));

    // Size
    m_labelMetaDataNewSizeX = new QLabel;
    m_labelMetaDataNewSizeX->setText(QString("Size X:"));
    m_labelMetaDataNewSizeY = new QLabel;
    m_labelMetaDataNewSizeY->setText(QString("Size Y:"));
    m_labelMetaDataNewSizeZ = new QLabel;
    m_labelMetaDataNewSizeZ->setText(QString("Size Z:"));
    m_lineEditMetaDataNewSizeX = new QLineEdit;
    m_lineEditMetaDataNewSizeX->setValidator(m_validatorSize);
    m_lineEditMetaDataNewSizeX->setText(QString::number(m_sizeOriginal.x()));
    m_lineEditMetaDataNewSizeY = new QLineEdit;
    m_lineEditMetaDataNewSizeY->setValidator(m_validatorSize);
    m_lineEditMetaDataNewSizeY->setText(QString::number(m_sizeOriginal.y()));
    m_lineEditMetaDataNewSizeZ = new QLineEdit;
    m_lineEditMetaDataNewSizeZ->setValidator(m_validatorSize);
    m_lineEditMetaDataNewSizeZ->setText(QString::number(m_sizeOriginal.z()));

    m_vLayoutmetaDataNewSizeX = new QHBoxLayout;
    m_vLayoutmetaDataNewSizeX->addWidget(m_labelMetaDataNewSizeX);
    m_vLayoutmetaDataNewSizeX->addWidget(m_lineEditMetaDataNewSizeX);
    m_metaDataNewSizeX = new QGroupBox;
    m_metaDataNewSizeX->setLayout(m_vLayoutmetaDataNewSizeX);

    m_vLayoutmetaDataNewSizeY = new QHBoxLayout;
    m_vLayoutmetaDataNewSizeY->addWidget(m_labelMetaDataNewSizeY);
    m_vLayoutmetaDataNewSizeY->addWidget(m_lineEditMetaDataNewSizeY);
    m_metaDataNewSizeY = new QGroupBox;
    m_metaDataNewSizeY->setLayout(m_vLayoutmetaDataNewSizeY);

    m_vLayoutmetaDataNewSizeZ = new QHBoxLayout;
    m_vLayoutmetaDataNewSizeZ->addWidget(m_labelMetaDataNewSizeZ);
    m_vLayoutmetaDataNewSizeZ->addWidget(m_lineEditMetaDataNewSizeZ);
    m_metaDataNewSizeZ = new QGroupBox;
    m_metaDataNewSizeZ->setLayout(m_vLayoutmetaDataNewSizeZ);

    m_vLayoutmetaDataNewSize = new QVBoxLayout;
    m_vLayoutmetaDataNewSize->addWidget(m_metaDataNewSizeX);
    m_vLayoutmetaDataNewSize->addWidget(m_metaDataNewSizeY);
    m_vLayoutmetaDataNewSize->addWidget(m_metaDataNewSizeZ);

    m_metaDataNewSize = new QGroupBox;
    m_metaDataNewSize->setTitle(QString("Size in pixel:"));
    m_metaDataNewSize->setLayout(m_vLayoutmetaDataNewSize);

    // Spacing
    m_labelMetaDataNewSpacingX = new QLabel;
    m_labelMetaDataNewSpacingX->setText(QString("Spacing X:"));
    m_labelMetaDataNewSpacingY = new QLabel;
    m_labelMetaDataNewSpacingY->setText(QString("Spacing Y:"));
    m_labelMetaDataNewSpacingZ = new QLabel;
    m_labelMetaDataNewSpacingZ->setText(QString("Spacing Z:"));
    m_lineEditMetaDataNewSpacingX = new QLineEdit;
    m_lineEditMetaDataNewSpacingX->setValidator(m_validatorSpacing);
    m_lineEditMetaDataNewSpacingX->setReadOnly(true);
    m_lineEditMetaDataNewSpacingX->setText(QString::number(m_spacingOriginal.x()));
    m_lineEditMetaDataNewSpacingY = new QLineEdit;
    m_lineEditMetaDataNewSpacingY->setValidator(m_validatorSpacing);
    m_lineEditMetaDataNewSpacingY->setReadOnly(true);
    m_lineEditMetaDataNewSpacingY->setText(QString::number(m_spacingOriginal.y()));
    m_lineEditMetaDataNewSpacingZ = new QLineEdit;
    m_lineEditMetaDataNewSpacingZ->setValidator(m_validatorSpacing);
    m_lineEditMetaDataNewSpacingZ->setReadOnly(true);
    m_lineEditMetaDataNewSpacingZ->setText(QString::number(m_spacingOriginal.z()));

    m_vLayoutmetaDataNewSpacingX = new QHBoxLayout;
    m_vLayoutmetaDataNewSpacingX->addWidget(m_labelMetaDataNewSpacingX);
    m_vLayoutmetaDataNewSpacingX->addWidget(m_lineEditMetaDataNewSpacingX);
    m_metaDataNewSpacingX = new QGroupBox;
    m_metaDataNewSpacingX->setLayout(m_vLayoutmetaDataNewSpacingX);

    m_vLayoutmetaDataNewSpacingY = new QHBoxLayout;
    m_vLayoutmetaDataNewSpacingY->addWidget(m_labelMetaDataNewSpacingY);
    m_vLayoutmetaDataNewSpacingY->addWidget(m_lineEditMetaDataNewSpacingY);
    m_metaDataNewSpacingY = new QGroupBox;
    m_metaDataNewSpacingY->setLayout(m_vLayoutmetaDataNewSpacingY);

    m_vLayoutmetaDataNewSpacingZ = new QHBoxLayout;
    m_vLayoutmetaDataNewSpacingZ->addWidget(m_labelMetaDataNewSpacingZ);
    m_vLayoutmetaDataNewSpacingZ->addWidget(m_lineEditMetaDataNewSpacingZ);
    m_metaDataNewSpacingZ = new QGroupBox;
    m_metaDataNewSpacingZ->setLayout(m_vLayoutmetaDataNewSpacingZ);

    m_vLayoutmetaDataNewSpacing = new QVBoxLayout;
    m_vLayoutmetaDataNewSpacing->addWidget(m_metaDataNewSpacingX);
    m_vLayoutmetaDataNewSpacing->addWidget(m_metaDataNewSpacingY);
    m_vLayoutmetaDataNewSpacing->addWidget(m_metaDataNewSpacingZ);

    m_metaDataNewSpacing = new QGroupBox;
    m_metaDataNewSpacing->setTitle(QString("Spacing in cm:"));
    m_metaDataNewSpacing->setLayout(m_vLayoutmetaDataNewSpacing);

    // metaDataNew
    m_hLayoutmetaDataNew = new QHBoxLayout;
    m_hLayoutmetaDataNew->addWidget(m_metaDataNewSize);
    m_hLayoutmetaDataNew->addWidget(m_metaDataNewSpacing);

    m_metaDataNew = new QGroupBox;
    m_metaDataNew->setTitle(QString("New size and spacing:"));
    m_metaDataNew->setLayout(m_hLayoutmetaDataNew);

    // Trigger Calculations
    connect(m_lineEditMetaDataNewSizeX, &QLineEdit::textChanged, this,
            &DialogResizeVolumeData::updateSpacingX);
    connect(m_lineEditMetaDataNewSizeY, &QLineEdit::textChanged, this,
            &DialogResizeVolumeData::updateSpacingY);
    connect(m_lineEditMetaDataNewSizeZ, &QLineEdit::textChanged, this,
            &DialogResizeVolumeData::updateSpacingZ);
}

void DialogResizeVolumeData::setupSectionTextureSize() {
    // Size
    m_labelTextureSizeOriginal = new QLabel;
    m_labelTextureSizeOriginal->setText(QString("Original: ") +
                                        QString::number(m_textureSizeOriginal) + " MB");
    m_labelTextureSizeNew = new QLabel;
    m_labelTextureSizeNew->setText(QString("New: ") + QString::number(m_textureSizeNew) + " MB");
    m_labelTextureSizeDriverMax = new QLabel;
    m_labelTextureSizeDriverMax->setText(QString("GPU Driver Maximum Support: ") +
                                         QString::number(m_textureSizeDriverMax) + " x " +
                                         QString::number(m_textureSizeDriverMax) + " x " +
                                         QString::number(m_textureSizeDriverMax) + " pixel");

    m_labelTotalGPUVRAM = new QLabel;
    m_labelTotalGPUVRAM->setText(
        QString("GPU Total VRAM: N.A. (Only NVIDIA GPU driver is supported.)"));
    m_labelAvailableGPUVRAM = new QLabel;
    m_labelAvailableGPUVRAM->setText(
        QString("GPU Available VRAM: N.A. (Only NVIDIA GPU driver is supported.)"));

    // textureSize
    m_hLayouttextureSize = new QVBoxLayout;
    m_hLayouttextureSize->addWidget(m_labelTextureSizeOriginal);
    m_hLayouttextureSize->addWidget(m_labelTextureSizeNew);
    m_hLayouttextureSize->addWidget(m_labelAvailableGPUVRAM);
    m_hLayouttextureSize->addWidget(m_labelTotalGPUVRAM);
    m_hLayouttextureSize->addWidget(m_labelTextureSizeDriverMax);

    m_textureSize = new QGroupBox;
    m_textureSize->setTitle(QString("OpenGL 3D Texture in Megabytes:"));
    m_textureSize->setLayout(m_hLayouttextureSize);
}

void DialogResizeVolumeData::setupSectionInterpolationMethod() {
    m_labelInterpolationMethod = new QLabel;
    m_labelInterpolationMethod->setText(QString("Interpolation Method:"));

    m_comboBoxInterpolationMethod = new QComboBox;
    m_comboBoxInterpolationMethod->addItems({"Fast & Low Quality [Nearest Neighbour]",
                                             "Slow & Medium Quality [Tri-Linear]",
                                             "Slowest & Best Quality [Tri-Cubic]"});
    m_comboBoxInterpolationMethod->setCurrentIndex(1);

    m_hLayoutInterpolationMethod = new QHBoxLayout;
    m_hLayoutInterpolationMethod->addWidget(m_labelInterpolationMethod);
    m_hLayoutInterpolationMethod->addWidget(m_comboBoxInterpolationMethod);

    m_groupInterpolationMethod = new QGroupBox;
    m_groupInterpolationMethod->setLayout(m_hLayoutInterpolationMethod);
}

void DialogResizeVolumeData::setupSectionOKAndCancel() {
    m_buttonOK = new QPushButton;
    m_buttonOK->setText(QString("OK"));

    m_buttonCancel = new QPushButton;
    m_buttonCancel->setText(QString("Cancel"));

    m_hLayoutOKAndCancel = new QHBoxLayout;
    m_hLayoutOKAndCancel->addWidget(m_buttonOK);
    m_hLayoutOKAndCancel->addWidget(m_buttonCancel);

    m_groupOKAndCancel = new QGroupBox;
    m_groupOKAndCancel->setLayout(m_hLayoutOKAndCancel);

    connect(m_buttonOK, &QPushButton::clicked, this, &DialogResizeVolumeData::onOKButtonClicked);
    connect(m_buttonCancel, &QPushButton::clicked, this,
            &DialogResizeVolumeData::onCancelButtonClicked);
}
void DialogResizeVolumeData::setupVRAMTimer() {
    m_timerVRAMupdate = new QTimer(this);
    connect(m_timerVRAMupdate, &QTimer::timeout, this, &DialogResizeVolumeData::requestVRAMinfoUpdate);
    m_timerVRAMupdate->start(250);
}
} // namespace VDS
