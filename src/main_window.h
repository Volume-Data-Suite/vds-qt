#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"

#include <VDTK/VolumeDataHandler.h>

#include "fileio/import_item_list.h"

namespace VDS {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = Q_NULLPTR);

public slots:
    void openImportRawDialog();
    void saveRecentFilesList();
    void loadRecentFilesList();
    void importRAW3D(const ImportItemRaw& item);

    void importRecentFile(std::size_t index);

    void updateFrametime(float frameTime, float renderEverything, float volumeRendering);

    void updateThresholdFromSlider(int threshold);

    void updateHistogram();

    void setValueWindowPreset(const QString& preset);

private:
    void updateVolumeData();
    void setupFileMenu();
    void refreshRecentFiles();

    Ui::MainWindowClass ui;

    // File Menu
    QMenu* m_menuFiles;
    QAction* m_actionImportRAW3D;
    QAction* m_actionImportBitmapSeries;
    QAction* m_actionImportBinarySlices;
    QMenu* m_menuRecentFiles;
    QAction* m_actionExportRAW3D;
    QAction* m_actionExportBitmapSeries;

    VDTK::VolumeDataHandler m_vdh;

    ImportItemList m_importList;
};
} // namespace VDS
