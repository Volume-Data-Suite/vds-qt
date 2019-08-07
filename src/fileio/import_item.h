#pragma once

#include <filesystem>
#include <QJsonObject>
#include <QString>
#include <QVector3D>

namespace VDS {
// Interface class for processing different kind of import item for the "last opened" section in
// import menu section
class ImportItem {
public:
    ~ImportItem() = default;

    // Some file types import a single file, other need a folder with multiple files
    virtual const QString getFileName() const = 0;
    virtual const QJsonObject serialize() const = 0;
    virtual void deserialize(const QJsonObject& json) = 0;
    const std::filesystem::path getFilePath() const;

protected:
    ImportItem(const std::filesystem::path& path);

    std::filesystem::path m_path;
};

class ImportItemRaw : public ImportItem {
public:
    ImportItemRaw(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                  const QVector3D& size, const QVector3D& spacing);
    ImportItemRaw();
    ~ImportItemRaw() = default;

    const QString getFileName() const override;
    const QVector3D getSize() const;
    const QVector3D getSpacing() const;
    uint8_t getBitsPerVoxel() const;

    const QJsonObject serialize() const;
    void deserialize(const QJsonObject& json);

private:
    uint8_t m_bitsPerVoxel;
    QVector3D m_size;
    QVector3D m_spacing;
};

} // namespace VDS