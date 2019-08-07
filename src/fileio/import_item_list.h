#pragma once
#include "import_item.h"

#include <QActionGroup>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>

namespace VDS {
enum class ImportType { RAW3D, BinarySlices, BitmapSlices };

class ImportItemListEntry {
public:
    ImportItemListEntry(const ImportItem* item, ImportType type);
    ImportItemListEntry();
    ~ImportItemListEntry();

    // const QString getFilePath() const;
    // const QString getFileName() const;
    // ImportType getType() const;
    // const QJsonObject getMetaData() const;

    const QJsonObject serialize() const;
    void deserialize(const QJsonObject& json);
    const ImportItem* const getItem() const;
    ImportType getType() const;

private:
    const ImportItem* m_item;
    ImportType m_type;
};

class ImportItemList {
public:
    ImportItemList();

    void addImportItem(const ImportItemListEntry* item);

    const QJsonDocument serialize() const;
    void deserialize(const QJsonDocument& json);

    int getSize() const;

    void clear();

    const ImportItemListEntry* const getEntry(std::size_t index) const;

private:
    QList<const ImportItemListEntry*> m_itemList;
};

} // namespace VDS