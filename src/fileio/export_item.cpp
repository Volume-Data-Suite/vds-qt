#include "export_item.h"

namespace VDS {
const std::filesystem::path ExportItem::getPath() const {
    return m_path;
}

ExportItem::ExportItem(const std::filesystem::path& path) : m_path{path} {}

ExportItemRaw::ExportItemRaw(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                             const bool little_endian)
    : ExportItem(filePath), m_bitsPerVoxel(bitsPerVoxel), m_littleEndian(little_endian) {}

uint8_t ExportItemRaw::getBitsPerVoxel() const {
    return m_bitsPerVoxel;
}
bool ExportItemRaw::representedInLittleEndian() const {
    return m_littleEndian;
}
} // namespace VDS
