#include "import_item.h"

const std::filesystem::path VDS::Import::ImportItem::getFilePath() const
{
	return m_path;
}

VDS::Import::ImportItem::ImportItem(const std::filesystem::path path) : m_path(path)
{
}


VDS::Import::ImportItemRaw::ImportItemRaw(const std::filesystem::path filePath, const uint8_t bitsPerVoxel, const QVector3D size, const QVector3D spacing)
	: ImportItem(filePath), m_bitsPerVoxel(bitsPerVoxel), m_size(size), m_spacing(spacing)
{
}

const QString VDS::Import::ImportItemRaw::getFileName() const
{
	return QString::fromStdString(m_path.filename().string());
}
