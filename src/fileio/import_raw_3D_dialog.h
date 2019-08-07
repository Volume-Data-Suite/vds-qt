#pragma once

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "import_item.h"

namespace VDS {

class DialogImportRAW3D : public QDialog {
    Q_OBJECT

public:
    DialogImportRAW3D(QWidget* parent = 0);

    const ImportItemRaw getImportItem() const;

public slots:
    void selectFile();
    void onOKButtonClicked();
    void onCancelButtonClicked();

private:
    bool checkCurrentInput();

    void setupSectionPathToFile();
    void setupSectionBitsPerVoxel();
    void setupSectionSize();
    void setupSectionSpacing();
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

    // Size
    QGroupBox* m_groupSize;
    QHBoxLayout* m_hLayoutSize;
    QVBoxLayout* m_vLayoutSize;
    QLabel* m_labelSize;
    QLineEdit* m_textSizeX;
    QLineEdit* m_textSizeY;
    QLineEdit* m_textSizeZ;
    QIntValidator* m_validatorSize;

    // Spacing
    QGroupBox* m_groupSpacing;
    QHBoxLayout* m_hLayoutSpacing;
    QVBoxLayout* m_vLayoutSpacing;
    QLabel* m_labelSpacing;
    QLineEdit* m_textSpacingX;
    QLineEdit* m_textSpacingY;
    QLineEdit* m_textSpacingZ;
    QRegExpValidator* m_validatorSpacing;

    // OK and Cancel
    QGroupBox* m_groupOKAndCancel;
    QHBoxLayout* m_hLayoutOKAndCancel;
    QPushButton* m_buttonOK;
    QPushButton* m_buttonCancel;
};
} // namespace VDS
