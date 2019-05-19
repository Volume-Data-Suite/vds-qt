#pragma once

#include <QString>
#include <filesystem>
#include <QVector3D>

namespace VDS::Import
{
	// Interface class for processing different kind of import item for the "last opened" section in import menu section
	class ImportItem
	{
	public:
		~ImportItem() = default;

		// Some file types import a single file, other need a folder with multiple files
		virtual const QString getFileName() const = 0;
		const std::filesystem::path getFilePath() const;

	protected:
		ImportItem(const std::filesystem::path path);

		const std::filesystem::path m_path;
	};

	class ImportItemRaw : protected ImportItem
	{
	public:
		ImportItemRaw(const std::filesystem::path filePath,
			const uint8_t bitsPerVoxel,
			const QVector3D size,
			const QVector3D spacing);
		~ImportItemRaw() = default;

		const QString getFileName() const override;

	private:
		const uint8_t m_bitsPerVoxel;
		const QVector3D m_size;
		const QVector3D m_spacing;

	};

}