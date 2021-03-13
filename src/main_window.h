#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_main_window.h"

#include <VDTK/VolumeDataHandler.h>

#include "fileio/import_item_list.h"
#include "fileio/import_item.h"
#include "fileio/export_item.h"

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
    void saveRecentFilesList();
    void loadRecentFilesList();
    void importRAW3D(const ImportItemRaw& item);

    void importRecentFile(std::size_t index);

    void openExportRawDialog();
    void exportRAW3D(const ExportItemRaw& item);

    void updateFrametime(float frameTime, float renderEverything, float volumeRendering);

    void updateThresholdFromSlider(int threshold);

    void computeHistogram();

    void setValueWindowPreset(const QString& preset);

    void errorRawExport();

signals:
    void updateHistogram(const std::vector<uint16_t>& histogram, bool ignoreBorders);
    // -1 = allow it, 0 = unchanged, 1 = do not allow it
    void updateUIPermissions(int read, int write);
    void showErrorExportRaw();

private:
    void updateVolumeData();
    void setupFileMenu();
    void refreshRecentFiles();

    bool checkIsBigEndian();

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

    std::atomic<int> readBlockCount;
    std::atomic<int> writeBlockCount;
};
} // namespace VDS
