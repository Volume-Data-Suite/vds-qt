#include "import_item.h"

namespace VDS {
const std::filesystem::path ImportItem::getFilePath() const {
    return m_path;
}

ImportItem::ImportItem(const std::filesystem::path& path) : m_path{path} {}

ImportItemRaw::ImportItemRaw(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                             const bool little_endian, const QVector3D& size,
                             const QVector3D& spacing)
    : ImportItem(filePath), m_bitsPerVoxel(bitsPerVoxel), m_littleEndian(little_endian),
      m_size(size), m_spacing(spacing) {}

ImportItemRaw::ImportItemRaw()
    : m_bitsPerVoxel{}, m_littleEndian{}, m_size{}, m_spacing{}, ImportItem{
                                                                     std::filesystem::path{}} {}

const QString ImportItemRaw::getFileName() const {
    return QString::fromStdString(m_path.filename().string());
}
const QVector3D ImportItemRaw::getSize() const {
    return m_size;
}
const QVector3D ImportItemRaw::getSpacing() const {
    return m_spacing;
}
uint8_t ImportItemRaw::getBitsPerVoxel() const {
    return m_bitsPerVoxel;
}
bool ImportItemRaw::representedInLittleEndian() const {
    return m_littleEndian;
}
const QJsonObject ImportItemRaw::serialize() const {
    // QJsonObject supports just a few data types
    QJsonObject jsonSize;
    jsonSize["x"] = static_cast<int>(m_size.x());
    jsonSize["y"] = static_cast<int>(m_size.y());
    jsonSize["z"] = static_cast<int>(m_size.z());

    QJsonObject jsonSpacing;
    jsonSpacing["x"] = static_cast<double>(m_spacing.x());
    jsonSpacing["y"] = static_cast<double>(m_spacing.y());
    jsonSpacing["z"] = static_cast<double>(m_spacing.z());

    QJsonObject json;
    json["path"] = QString(m_path.string().c_str());
    json["bitPerVoxel"] = m_bitsPerVoxel;
    json["littleEndian"] = m_littleEndian;
    json["size"] = jsonSize;
    json["spacing"] = jsonSpacing;

    return json;
}
void ImportItemRaw::deserialize(const QJsonObject& json) {
    QJsonObject jsonSize = json["size"].toObject();
    m_size.setX(static_cast<float>(jsonSize["x"].toInt()));
    m_size.setY(static_cast<float>(jsonSize["y"].toInt()));
    m_size.setZ(static_cast<float>(jsonSize["z"].toInt()));

    QJsonObject jsonSpacing = json["spacing"].toObject();
    m_spacing.setX(static_cast<float>(jsonSpacing["x"].toDouble()));
    m_spacing.setY(static_cast<float>(jsonSpacing["y"].toDouble()));
    m_spacing.setZ(static_cast<float>(jsonSpacing["z"].toDouble()));

    m_path = std::filesystem::path(json["path"].toString().toStdString());

    m_bitsPerVoxel = static_cast<uint8_t>(json["bitPerVoxel"].toInt());
    m_littleEndian = static_cast<uint8_t>(json["littleEndian"].toBool());
}
ImportItemBinarySlices::ImportItemBinarySlices(const std::filesystem::path& directoryPath,
                                               const uint8_t bitsPerVoxel, const bool little_endian,
                                               const VDTK::VolumeAxis axis,
                                               const QVector3D& size, const QVector3D& spacing)
    : ImportItemRaw(directoryPath, bitsPerVoxel, little_endian, size, spacing), m_axis(axis) {}

ImportItemBinarySlices::ImportItemBinarySlices() : ImportItemRaw() {}

const VDTK::VolumeAxis ImportItemBinarySlices::getAxis() const {
    return m_axis;
}

const QJsonObject ImportItemBinarySlices::serialize() const {
    QJsonObject json = ImportItemRaw::serialize();
    json["axis"] = static_cast<int>(m_axis);
    return json;
}
void ImportItemBinarySlices::deserialize(const QJsonObject& json) {
    ImportItemRaw::deserialize(json);
    m_axis = static_cast<VDTK::VolumeAxis>(json["axis"].toInt());
}
} // namespace VDS
