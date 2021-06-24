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

#include "export_item.h"

namespace VDS {

struct ValueWindow {
    ValueWindow() = default;

    ValueWindow(const ValueWindow& valueWindow) {
        this->function = valueWindow.function;
        this->width = valueWindow.width;
        this->center = valueWindow.center;
        this->offset = valueWindow.offset;
    }
    ValueWindow(const QString& function, int width, int center, int offset) {
        this->function = function;
        this->width = width;
        this->center = center;
        this->offset = offset;
    }

    QString function = QString("");
    int width = 0;
    int center = 0;
    int offset = 0;
};

class DialogExportRAW3D : public QDialog {
    Q_OBJECT

public:
    DialogExportRAW3D(const ValueWindow& valueWindow, const QVector3D& size, const QVector3D& spacing,
                      QWidget* parent = 0);

    const ExportItemRaw getExportItem() const;

public slots:
    void selectFile();
    void deactiveSectionsIfNecessary(const int bitsPerVoxelIndex);
    void onOKButtonClicked();
    void onCancelButtonClicked();

private:
    bool checkCurrentInput();
    bool checkIsBigEndian();

    void setupSectionPathToFile();
    void setupSectionMetaData();
    void setupSectionValueWindow();
    void setupSectionBitsPerVoxel();
    void setupSectionEndianess();
    void setupSectionOKAndCancel();

    // Dialog Window
    QVBoxLayout* m_vLayoutDialog;

    // Path to file
    QGroupBox* m_groupPathToFile;
    QHBoxLayout* m_hLayoutPathToFile;
    QVBoxLayout* m_vLayoutPathToFile;
    QLabel* m_labelPathToFile;
    QLineEdit* m_textPathToFile;
    QPushButton* m_buttonPathToFile;

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

    // Value window
    QGroupBox* m_valueWindowGroup;
    QVBoxLayout* m_vLayoutValueWindow;
    QLabel* m_labelValueWindowInfo;
    QLabel* m_labelValueWindowFunction;
    QLabel* m_labelValueWindowWidth;
    QLabel* m_labelValueWindowCenter;
    QLabel* m_labelValueWindowOffset;
    QSpacerItem* m_verticalSpacer;
    ValueWindow m_valueWindow;

    // Bits per voxel
    QGroupBox* m_groupBitsPerVoxel;
    QHBoxLayout* m_hLayoutBitsPerVoxel;
    QLabel* m_labelBitsPerVoxel;
    QComboBox* m_comboBoxBitsPerVoxelOptions;

    // Endianess
    QGroupBox* m_groupEndianess;
    QHBoxLayout* m_hLayoutEndianess;
    QLabel* m_labelEndianess;
    QComboBox* m_comboBoxEndianessOptions;

    // OK and Cancel
    QGroupBox* m_groupOKAndCancel;
    QHBoxLayout* m_hLayoutOKAndCancel;
    QPushButton* m_buttonOK;
    QPushButton* m_buttonCancel;
};
} // namespace VDS
