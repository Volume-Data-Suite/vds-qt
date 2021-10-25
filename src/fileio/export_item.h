#pragma once

#include <filesystem>

#include <VDTK//common/CommonDataTypes.h>

namespace VDS {
// Interface class for processing different kind of import item for the "last opened" section in
// import menu section
class ExportItem {
public:
    ~ExportItem() = default;

    // Some file types import a single file, other need a folder with multiple files
    const std::filesystem::path getPath() const;

protected:
    ExportItem(const std::filesystem::path& path);

    std::filesystem::path m_path;
};

class ExportItemRaw : public ExportItem {
public:
    ExportItemRaw(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                  const bool little_endian, const bool applyWindow);
    ~ExportItemRaw() = default;

    uint8_t getBitsPerVoxel() const;
    bool representedInLittleEndian() const;
    bool applyValueWindow() const;

private:
    uint8_t m_bitsPerVoxel;
    bool m_littleEndian;
    bool m_applyWindow;
};

class ExportItemImageSeries : public ExportItem {
public:
    ExportItemImageSeries(const std::filesystem::path& directoryPath, const bool applyWindow);
    ~ExportItemImageSeries() = default;

    bool applyValueWindow() const;

private:
    bool m_applyWindow;
};

} // namespace VDS