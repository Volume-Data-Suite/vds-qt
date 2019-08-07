#include "import_item_list.h"

namespace VDS {
ImportItemListEntry::ImportItemListEntry(const ImportItem* item, ImportType type)
    : m_item{item}, m_type{type} {}

ImportItemListEntry::ImportItemListEntry() : m_item{}, m_type{} {}

ImportItemListEntry::~ImportItemListEntry() {
    delete m_item;
}

const QJsonObject ImportItemListEntry::serialize() const {
    QJsonObject json;
    json["type"] = static_cast<int>(m_type);
    json["importItem"] = m_item->serialize();
    return json;
}

void ImportItemListEntry::deserialize(const QJsonObject& json) {
    const QJsonObject item = json["importItem"].toObject();
    const ImportType type = static_cast<ImportType>(json["type"].toInt());

    switch (type) {
    case VDS::ImportType::RAW3D: {
        ImportItemRaw* importItem = new ImportItemRaw();
        importItem->deserialize(item);
        delete m_item;
        m_item = importItem;
        break;
    }
    case VDS::ImportType::BinarySlices: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    case VDS::ImportType::BitmapSlices: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    default: {
        Q_UNIMPLEMENTED();
        return;
        break;
    }
    }

    m_type = type;
}

const ImportItem* const ImportItemListEntry::getItem() const {
    return m_item;
}

ImportType ImportItemListEntry::getType() const {
    return m_type;
}

ImportItemList::ImportItemList() {}

void ImportItemList::addImportItem(const ImportItemListEntry* item) {
    // remove existing item with same path
    for (int index = 0; index < m_itemList.size(); index++) {
        const ImportItemListEntry* existingItem = m_itemList.at(index);
        if (existingItem->getItem()->getFilePath() == item->getItem()->getFilePath()) {
            m_itemList.removeAt(index);
        }
    }

    m_itemList.push_front(item);
}
const QJsonDocument ImportItemList::serialize() const {
    QJsonArray jsonImportItems;
    for (const auto& item : m_itemList) {
        jsonImportItems.append(item->serialize());
    }

    QJsonObject json;
    json["importItems"] = jsonImportItems;

    return QJsonDocument(json);
}
void ImportItemList::deserialize(const QJsonDocument& json) {
    const QJsonArray jsonImportItems = json.object()["importItems"].toArray();

    for (const auto& item : jsonImportItems) {
        const QJsonObject importItem = item.toObject();
        ImportItemListEntry* entry = new ImportItemListEntry();
        entry->deserialize(importItem);
        m_itemList.push_back(entry);
    }
}

int ImportItemList::getSize() const {
    return m_itemList.size();
}

void ImportItemList::clear() {
    m_itemList.clear();
}

const ImportItemListEntry* const ImportItemList::getEntry(std::size_t index) const {
    return m_itemList.at(static_cast<int>(index));
}

} // namespace VDS
