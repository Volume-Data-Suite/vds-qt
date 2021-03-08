#pragma once

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "export_item.h"

namespace VDS {

class DialogExportRAW3D : public QDialog {
    Q_OBJECT

public:
    DialogExportRAW3D(QWidget* parent = 0);

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
