#pragma once

#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "import_item.h"

namespace VDS {

class DialogImportBinarySlices : public QDialog {
    Q_OBJECT

public:
    DialogImportBinarySlices(QWidget* parent = 0);

    const ImportItemBinarySlices getImportItem() const;

public slots:
    void selectDirectory();
    void onOKButtonClicked();
    void onCancelButtonClicked();
    void updateSizeDependingOnFileCount();

private:
    bool checkCurrentInput();
    bool checkIsBigEndian();

    void setupSectionPathToFile();
    void setupSectionBitsPerVoxel();
    void setupSectionEndianess();
    void setupSectionSize();
    void setupSectionSpacing();
    void setupImportOrderPreview();
    void setupAxis();
    void setupSectionOKAndCancel();

    void previewImportOrder();

    // Dialog Window
    QVBoxLayout* m_vLayoutDialog;

    // Path to file
    QGroupBox* m_groupPathToDirectory;
    QHBoxLayout* m_hLayoutPathToDirectory;
    QVBoxLayout* m_vLayoutPathToDirectory;
    QLabel* m_labelPathToDirectory;
    QLineEdit* m_textPathToDirectory;
    QPushButton* m_buttonPathToDirectory;

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

    // Axis
    QGroupBox* m_groupAxis;
    QHBoxLayout* m_hLayoutAxis;
    QLabel* m_labelAxis;
    QComboBox* m_comboBoxAxis;

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

    // File Import Order Preview
    QGroupBox* m_groupFileImportPreview;
    QVBoxLayout* m_vLayoutFileImportPreview;
    QLabel* m_labelFileImportPreview;
    QTextEdit* m_textFileImportPreview;

    // OK and Cancel
    QGroupBox* m_groupOKAndCancel;
    QHBoxLayout* m_hLayoutOKAndCancel;
    QPushButton* m_buttonOK;
    QPushButton* m_buttonCancel;

    std::uint16_t m_numberOfSlices;
};
} // namespace VDS
