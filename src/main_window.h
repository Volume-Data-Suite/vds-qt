#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include "ui_main_window.h"

#include <VDTK/VolumeDataHandler.h>

#include "fileio/import_item_list.h"
#include "fileio/import_item.h"
#include "fileio/export_item.h"
#include "widgets/expandable_section_widget.h"

namespace VDS {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = Q_NULLPTR);

public slots:
    // when running a tasks that modifies the volume data on different threads
    // -1 = allow it, 0 = unchanged, 1 = do not allow it
    void setUIPermissions(int read, int write);


    void openImportRawDialog();
    void openImportBinarySlicesDialog();
    void saveRecentFilesList();
    void loadRecentFilesList();
    void refreshRecentFileList();
    void importRAW3D(const ImportItemRaw& item3D);
    void importBinarySlices(const ImportItemBinarySlices& item3D);

    void importRecentFile(std::size_t index);

    void openExportRawDialog();
    void exportRAW3D(const ExportItemRaw& item);
    void openExportImageSeriesDialog();
    void exportImageSeries(const ExportItemImageSeries& item);

    void openVolumeDataResizeDialog();

    void updateFrametime(float frameTime, float renderEverything, float volumeRendering);

    void updateThresholdFromSlider(int threshold);

    void resizeVolumeData(QVector3D newSize, int interpolationMethod);

    void computeHistogram();

    void setValueWindowPreset(const QString& preset);

    void setVertexDebugShaderEditor(const QString& vertexShader);
    void setFragmentDebugShaderEditor(const QString& fragmentShader);
    void triggerManualVertexShaderUpdateFromEditor();
    void triggerManualFragmentShaderUpdateFromEditor();

    void errorRawExport();
    void errorImageSeriesExport();
    void errorRawImport();
    void errorBinarySlicesImport();

    void toggleSliceViewEnabled();
    void toggleControllViewEnabled();

signals:
    void updateHistogram(const std::vector<uint16_t>& histogram, bool ignoreBorders);
    // -1 = allow it, 0 = unchanged, 1 = do not allow it
    void updateUIPermissions(int read, int write);
    void showErrorExportRaw();
    void showErrorExportImagesSeries();
    void showErrorImportRaw();
    void showErrorImportBinarySlices();
    void updateRecentFiles();
    void updateVertexShaderFromEditor(const QString& vertexShader);
    void updateFragmentShaderFromEditor(const QString& fragmentShader);
    void updateVolumeView(const std::array<std::size_t, 3> size, const std::array<float, 3> spacing,
                          const std::vector<uint16_t>& volumeData);

private:
    void updateVolumeData();
    void setupFileMenu();
    void setupViewMenu();
    void setupToolsMenu();
    void setupRendererView();
    void setupShaderEditor();

    bool checkIsBigEndian();

    Ui::MainWindowClass ui;

    // File Menu
    QMenu* m_menuFiles;
    QAction* m_actionImportRAW3D;
    QAction* m_actionImportBinarySlices;
    QMenu* m_menuRecentFiles;
    QAction* m_actionExportRAW3D;
    QAction* m_actionExportBitmapSeries;
    ImportItemList m_importList;

    // View Menu
    QMenu* m_menuView;
    QAction* m_actionResetView;
    QAction* m_actionToggleControlView;
    QAction* m_actionToggleSliceView;

    // Tools Menu
    QMenu* m_menuTools;
    QAction* m_actionResizeVolumeData;

    // Renderer View
    QLabel* m_labelSliceRendererX;
    QLabel* m_labelSliceRendererY;
    QLabel* m_labelSliceRendererZ;
    QLabel* m_labelRenderer;
    QSlider* m_sliderSliceRendererX;
    QSlider* m_sliderSliceRendererY;
    QSlider* m_sliderSliceRendererZ;
    QHBoxLayout* m_sliderSliceRendererXLayout;
    QHBoxLayout* m_sliderSliceRendererYLayout;
    QHBoxLayout* m_sliderSliceRendererZLayout;

    // Debug Shader Editor
    QLabel* m_shaderEditorInfo;
    QTextEdit* m_vertexShaderEdit;
    QVBoxLayout* m_vertexShaderEditLayout;
    ExpandableSectionWidget* m_vertexShaderEditorSection;
    QPushButton* m_buttonApplyVertexShader;
    QTextEdit* m_fragmentShaderEdit;
    QVBoxLayout* m_fragmentShaderEditLayout;
    ExpandableSectionWidget* m_fragmentShaderEditorSection;
    QPushButton* m_buttonApplyFragmentShader;
    QVBoxLayout* m_groupBoxShaderEditorLayout;


    VDTK::VolumeDataHandler m_vdh;

    std::atomic<int> readBlockCount;
    std::atomic<int> writeBlockCount;
};
} // namespace VDS
