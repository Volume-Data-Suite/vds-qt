#include "import_item.h"

namespace VDS
{
const std::filesystem::path ImportItem::getFilePath() const
{
	return m_path;
}

ImportItem::ImportItem(const std::filesystem::path path) : m_path(path)
{
}

ImportItemRaw::ImportItemRaw(const std::filesystem::path filePath, const uint8_t bitsPerVoxel, const QVector3D size, const QVector3D spacing)
	: ImportItem(filePath), m_bitsPerVoxel(bitsPerVoxel), m_size(size), m_spacing(spacing)
{
}

const QString ImportItemRaw::getFileName() const
{
	return QString::fromStdString(m_path.filename().string());
}
const QVector3D ImportItemRaw::getSize()
{
	return m_size;
}
const QVector3D ImportItemRaw::getSpacing()
{
	return m_spacing;
}
uint8_t ImportItemRaw::getBitsPerVoxel()
{
	return m_bitsPerVoxel;
}
}
