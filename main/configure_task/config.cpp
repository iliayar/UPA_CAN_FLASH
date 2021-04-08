#include "config.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

DataConfig::DataConfig() {
    QFile json_file;
    json_file.setFileName("configuration.json"); // FIXME
    json_file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString json_data = json_file.readAll();
    json_file.close();

    // qDebug() << json_data;

    QJsonDocument json = QJsonDocument::fromJson(json_data.toUtf8());

    if(!json.isArray()) {
        return;
    }

    for(const QJsonValue& group : json.array()) {
        if(!group.isObject()) {
            return;
        }
        const QJsonObject& json_group = group.toObject();
        std::string group_name = json_group["name"].toString().toStdString();
        std::vector<Field*> group_list{};
        for(const QJsonValue& field : json_group["fields"].toArray()) {
            const QJsonObject& json_field = field.toObject();
            std::string type = json_field["type"].toString().toStdString();
            std::string name = json_field["name"].toString().toStdString();
            uint16_t did = json_field["did"].toString().toUShort(nullptr, 0);
            int size;
            if (type == "string") {
                size = json_field["size"].toInt();
                group_list.push_back(new StringField(name, did, size));
            } else if(type == "int") {
                size = json_field["size"].toInt();
                group_list.push_back(new IntField(name, did, size));
            } else if(type == "enum") {
                int size = json_field["size"].toInt();
                std::vector<std::pair<std::string, uint64_t>> variants;
                for(const QJsonValue& variant : json_field["variants"].toArray()) {
                    const QJsonObject json_variant = variant.toObject();
                    std::string variant_name = json_variant["mnemonic"].toString().toStdString();
                    uint64_t value = json_variant["value"].toString().toULong(nullptr, 0);
                    variants.emplace_back(variant_name, value);
                }
                group_list.push_back(new EnumField(name, did, std::move(variants), size));
            } else if(type == "combo") {
                std::vector<MultiFieldItem> items{};
                size = 0;
                for(const QJsonValue& multi_field : json_field["fields"].toArray()) {
                    const QJsonObject json_multi_field = multi_field.toObject();
                    std::string item_name = json_multi_field["name"].toString().toStdString();
                    int item_size = json_multi_field["size"].toInt();
                    size += item_size;
                    items.push_back({item_size, item_name});
                }
                group_list.push_back(new MultiField(name, did, std::move(items)));
            } else if(type == "bytes") {
                size = json_field["size"].toInt();
                group_list.push_back(new VecField(name, did, size));
            }
            Can::Data::register_did(did, size);
        }
        fields.emplace_back(group_name, std::move(group_list));
    }
}
