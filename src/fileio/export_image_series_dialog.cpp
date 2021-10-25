
#include <QFileDialog>
#include <QMessageBox>
#include "export_image_series_dialog.h"

#include <limits>
#include <string>


#include "VDTK/common/CommonIO.h"

namespace VDS {

DialogExportImageSeries::DialogExportImageSeries(const QVector3D& size,
                                     const QVector3D& spacing,
                                     QWidget* parent)
    : QDialog(parent), m_size(size), m_spacing(spacing) {
    setWindowTitle(QString("Export Image Series"));

    // disable the context help button
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setupSectionPathToDirectory();
    setupSectionMetaData();
    setupSectionOKAndCancel();

    m_vLayoutDialog = new QVBoxLayout(this);
    m_vLayoutDialog->addWidget(m_metaData);
    m_vLayoutDialog->addWidget(m_groupPathToDirectory);
    m_vLayoutDialog->addWidget(m_groupOKAndCancel);

    setLayout(m_vLayoutDialog);

    // surpress setGeometry warning. Probably related to some weird stylesheet parsing.
    setMinimumSize(443, 275);
}

const ExportItemImageSeries DialogExportImageSeries::getExportItem() const {
    const std::filesystem::path path =
        std::filesystem::path(m_textPathToDirectory->text().toStdString());

    const bool applyValueWindow = false;

    return ExportItemImageSeries(path, applyValueWindow);
}

void DialogExportImageSeries::onOKButtonClicked() {
    if (checkCurrentInput()) {
        this->accept();
    } else {
        QMessageBox msgBox(QMessageBox::Warning, "Incomplete Image Series Export!",
                           "Some required fields are empty!");
        msgBox.exec();
    }
}
void DialogExportImageSeries::onCancelButtonClicked() {
    this->reject();
}
bool DialogExportImageSeries::checkCurrentInput() {
    if (m_textPathToDirectory->text().isEmpty()) {
        return false;
    }
    return true;
}
bool DialogExportImageSeries::checkIsBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}
void DialogExportImageSeries::setupSectionPathToDirectory() {
    m_labelPathToDirectory = new QLabel;
    m_labelPathToDirectory->setText(QString("Path to export directory:"));

    m_textPathToDirectory = new QLineEdit;
    m_textPathToDirectory->setReadOnly(true);
    m_textPathToDirectory->setMinimumWidth(350);
    m_textPathToDirectory->setPlaceholderText(QString("Select a directory to store the generated images."));

    m_buttonPathToDirectory = new QPushButton;
    m_buttonPathToDirectory->setText(QString("Select"));

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

    connect(m_buttonPathToDirectory, &QPushButton::clicked, this, &DialogExportImageSeries::selectDirectory);
}

void DialogExportImageSeries::setupSectionMetaData() {
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


void DialogExportImageSeries::setupSectionOKAndCancel() {
    m_buttonOK = new QPushButton;
    m_buttonOK->setText(QString("OK"));

    m_buttonCancel = new QPushButton;
    m_buttonCancel->setText(QString("Cancel"));

    m_hLayoutOKAndCancel = new QHBoxLayout;
    m_hLayoutOKAndCancel->addWidget(m_buttonOK);
    m_hLayoutOKAndCancel->addWidget(m_buttonCancel);

    m_groupOKAndCancel = new QGroupBox;
    m_groupOKAndCancel->setLayout(m_hLayoutOKAndCancel);

    connect(m_buttonOK, &QPushButton::clicked, this, &DialogExportImageSeries::onOKButtonClicked);
    connect(m_buttonCancel, &QPushButton::clicked, this, &DialogExportImageSeries::onCancelButtonClicked);
}

void DialogExportImageSeries::selectDirectory() {
    const QString path = QFileDialog::getExistingDirectory(
        nullptr, tr("Open Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (path.isEmpty()) {
        // Open file dialog got cancelled by user. Do not overwrite current text in case user open
        // file dialog a second time accidentally
        return;
    }
    m_textPathToDirectory->setText(path);
}
} // namespace VDS
