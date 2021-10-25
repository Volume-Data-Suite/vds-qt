#pragma once

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector3D>
#include <QTextEdit>

#include "export_item.h"

namespace VDS {

class DialogExportImageSeries : public QDialog {
    Q_OBJECT

public:
    DialogExportImageSeries(const QVector3D& size, const QVector3D& spacing,
                      QWidget* parent = 0);

    const ExportItemImageSeries getExportItem() const;

public slots:
    void selectDirectory();
    void onOKButtonClicked();
    void onCancelButtonClicked();

private:
    bool checkCurrentInput();
    bool checkIsBigEndian();

    void setupSectionPathToDirectory();
    void setupSectionMetaData();
    void setupSectionOKAndCancel();

    // Dialog Window
    QVBoxLayout* m_vLayoutDialog;

    // Metadata
    QGroupBox* m_metaData;
    QGroupBox* m_metaDataSize;
    QGroupBox* m_metaDataSpacing;
    QHBoxLayout* m_hLayoutMetaData;
    QVBoxLayout* m_vLayoutMetaDataSize;
    QVBoxLayout* m_vLayoutMetaDataSpacing;
    QLabel* m_labelmetaDataSizeX;
    QLabel* m_labelmetaDataSizeY;
    QLabel* m_labelmetaDataSizeZ;
    QLabel* m_labelmetaDataSpacingX;
    QLabel* m_labelmetaDataSpacingY;
    QLabel* m_labelmetaDataSpacingZ;
    const QVector3D m_size;
    const QVector3D m_spacing;

    // Path to file
    QGroupBox* m_groupPathToDirectory;
    QHBoxLayout* m_hLayoutPathToDirectory;
    QVBoxLayout* m_vLayoutPathToDirectory;
    QLabel* m_labelPathToDirectory;
    QLineEdit* m_textPathToDirectory;
    QPushButton* m_buttonPathToDirectory;

    // OK and Cancel
    QGroupBox* m_groupOKAndCancel;
    QHBoxLayout* m_hLayoutOKAndCancel;
    QPushButton* m_buttonOK;
    QPushButton* m_buttonCancel;
};
} // namespace VDS
