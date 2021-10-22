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
#include <QTimer>

namespace VDS {

class DialogResizeVolumeData : public QDialog {
    Q_OBJECT

public:
    DialogResizeVolumeData(const QVector3D& size, const QVector3D& spacing,
                           const int textureSizeMax, QWidget* parent = 0);
    
    QVector3D getNewSize() const;
    int getInterploationMethod() const;

public slots:
    void onOKButtonClicked();
    void onCancelButtonClicked();
    void computeTextureSizeOriginal();
    void computeTextureSizeNew();
    void updateSpacingX();
    void updateSpacingY();
    void updateSpacingZ();
    void recieveVRAMinfoUpdate(bool success, int dedicatedMemory, int totalAvailableMemory,
                               int availableDedicatedMemory, int envictionCount,
                               int envictedMemory);

signals:
    void requestVRAMinfoUpdate();

private:
    bool checkCurrentInput();

    void setupSectionmetaDataOriginal();
    void setupSectionMetaDataNew();
    void setupSectionTextureSize();
    void setupSectionInterpolationMethod();
    void setupSectionOKAndCancel();
    void setupVRAMTimer();
    void updateVRAMLabels();

    // Dialog Window
    QVBoxLayout* m_vLayoutDialog;

    // Original Meta Data
    const QVector3D m_sizeOriginal;
    const QVector3D m_spacingOriginal;
    QGroupBox* m_metaDataOriginal;
    QGroupBox* m_metaDataOriginalSize;
    QGroupBox* m_metaDataOriginalSpacing;
    QHBoxLayout* m_hLayoutmetaDataOriginal;
    QVBoxLayout* m_vLayoutmetaDataOriginalSize;
    QVBoxLayout* m_vLayoutmetaDataOriginalSpacing;
    QLabel* m_labelMetaDataOriginalSizeX;
    QLabel* m_labelMetaDataOriginalSizeY;
    QLabel* m_labelMetaDataOriginalSizeZ;
    QLabel* m_labelMetaDataOriginalSpacingX;
    QLabel* m_labelMetaDataOriginalSpacingY;
    QLabel* m_labelMetaDataOriginalSpacingZ;

    // New Meta Data
    QGroupBox* m_metaDataNew;
    QGroupBox* m_metaDataNewSize;
    QGroupBox* m_metaDataNewSpacing;
    QGroupBox* m_metaDataNewSizeX;
    QGroupBox* m_metaDataNewSizeY;
    QGroupBox* m_metaDataNewSizeZ;
    QGroupBox* m_metaDataNewSpacingX;
    QGroupBox* m_metaDataNewSpacingY;
    QGroupBox* m_metaDataNewSpacingZ;
    QHBoxLayout* m_hLayoutmetaDataNew;
    QVBoxLayout* m_vLayoutmetaDataNewSize;
    QHBoxLayout* m_vLayoutmetaDataNewSizeX;
    QHBoxLayout* m_vLayoutmetaDataNewSizeY;
    QHBoxLayout* m_vLayoutmetaDataNewSizeZ;
    QVBoxLayout* m_vLayoutmetaDataNewSpacing;
    QHBoxLayout* m_vLayoutmetaDataNewSpacingX;
    QHBoxLayout* m_vLayoutmetaDataNewSpacingY;
    QHBoxLayout* m_vLayoutmetaDataNewSpacingZ;
    QLineEdit* m_lineEditMetaDataNewSizeX;
    QLineEdit* m_lineEditMetaDataNewSizeY;
    QLineEdit* m_lineEditMetaDataNewSizeZ;
    QLineEdit* m_lineEditMetaDataNewSpacingX;
    QLineEdit* m_lineEditMetaDataNewSpacingY;
    QLineEdit* m_lineEditMetaDataNewSpacingZ;
    QLabel* m_labelMetaDataNewSizeX;
    QLabel* m_labelMetaDataNewSizeY;
    QLabel* m_labelMetaDataNewSizeZ;
    QLabel* m_labelMetaDataNewSpacingX;
    QLabel* m_labelMetaDataNewSpacingY;
    QLabel* m_labelMetaDataNewSpacingZ;
    QIntValidator* m_validatorSize;
    QRegExpValidator* m_validatorSpacing;

    // Interpolation Method
    QGroupBox* m_groupInterpolationMethod;
    QHBoxLayout* m_hLayoutInterpolationMethod;
    QLabel* m_labelInterpolationMethod;
    QComboBox* m_comboBoxInterpolationMethod;

    // Texture Size
    QGroupBox* m_textureSize;
    QVBoxLayout* m_hLayouttextureSize;
    QLabel* m_labelTextureSizeOriginal;
    QLabel* m_labelTextureSizeNew;
    QLabel* m_labelTextureSizeDriverMax;
    QLabel* m_labelTotalGPUVRAM;
    QLabel* m_labelAvailableGPUVRAM;
    // Size in MB
    float m_availableGPUVRAM;
    float m_textureSizeOriginal;
    float m_textureSizeNew;
    float m_textureSizeDriverMax;

    // VRAM update
    QTimer* m_timerVRAMupdate;

    // OK and Cancel
    QGroupBox* m_groupOKAndCancel;
    QHBoxLayout* m_hLayoutOKAndCancel;
    QPushButton* m_buttonOK;
    QPushButton* m_buttonCancel;
};
} // namespace VDS
