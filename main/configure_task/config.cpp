#include "config.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QCoreApplication>

#include "fields.h"

DataConfig::DataConfig() {
    QString app_path = QCoreApplication::applicationDirPath();
    {
        QFile json_file;
        json_file.setFileName(app_path + "/configuration.json"); // FIXME
        json_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString json_data = json_file.readAll();
        json_file.close();

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
                    size = json_field["size"].toInt();
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
    
    {
        QFile json_file;
        json_file.setFileName(app_path + "/configuration_err.json"); // FIXME
        json_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString json_data = json_file.readAll();
        json_file.close();

        QJsonDocument json = QJsonDocument::fromJson(json_data.toUtf8());

        if(!json.isArray()) {
            return;
        }

        for(const QJsonValue& err_val : json.array()) {
            const QJsonObject& err = err_val.toObject();
            uint16_t code = err["code"].toString().toUInt(nullptr, 0);
            std::string name = err["name"].toString().toStdString();
            errors.insert({code, {name, {}}});
            auto& var_map = errors[code].second;
            const QJsonValue& vars_val = err["variants"];
            for (const QJsonValue& var_val : vars_val.toArray()) {
                const QJsonObject& var = var_val.toObject();
                std::string mnemonic = var["mnemonic"].toString().toStdString();
                std::string description =
                    var["description"].toString().toStdString();
                uint8_t type = var["type"].toString().toUShort(nullptr, 0);
                var_map.insert({type, {mnemonic, description}});
            }
        }
    }
    
}

void DataConfig::group_to_json(QString filename, std::string group) {
    QJsonObject obj;
    for (auto& [name, fields] : this->fields) {
        if (name == group) {
            for (Field* field : fields) {
                obj.insert(field->get_name(), field->to_string());
            }
        }
    }
    QFile json_file;
    json_file.setFileName(filename);
    json_file.open(QIODevice::Text | QIODevice::WriteOnly);
    if(!json_file.isOpen()) {
        return; // FIXME
    }
    json_file.write(QJsonDocument(obj).toJson());
    json_file.close();
}

void DataConfig::json_to_group(QString filename, std::string group) {
    QFile json_file;
    json_file.setFileName(filename);
    json_file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(!json_file.isOpen()) {
        return; // FIXME
    }
    QString json_data = json_file.readAll();
    json_file.close();

    QJsonDocument json = QJsonDocument::fromJson(json_data.toUtf8());
    QJsonObject obj = json.object();
    for (auto& [name, fields] : this->fields) {
        if (name == group) {
            for (Field* field : fields) {
                if(!obj.contains(field->get_name())) {
                    continue;
                }
                field->from_string(obj[field->get_name()].toString());
            }
        }
    }
}
