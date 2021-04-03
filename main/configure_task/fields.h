#pragma once

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QTextEdit>
#include <QSpinBox>
#include <QDialog>
#include <QComboBox>

#include "service_all.h"
#include "task.h"

class Field : public QGroupBox {
    Q_OBJECT
public:
    Field(QWidget* parent, std::string name, Can::DataIdentifier id)
        : QGroupBox(QString::fromStdString(name), parent), m_id(id) {
    }
    Field(std::string name, Can::DataIdentifier id) : Field(nullptr, name, id) {
    }

    void init(ConfigurationTask* task) {
        m_task = task;
        m_layout = new QHBoxLayout(this);
        create_fields();
        QFrame* frame = new QFrame(this);
        QHBoxLayout* layout = new QHBoxLayout(frame);
        QPushButton* write_btn = new QPushButton(tr("&Write"));
        QPushButton* read_btn = new QPushButton(tr("&Read"));
        layout->addWidget(write_btn);
        layout->addWidget(read_btn);
        m_layout->addWidget(frame);
        connect(read_btn, &QPushButton::pressed, this, &Field::read);
        connect(write_btn, &QPushButton::pressed, this, &Field::write);
    }
public slots:
    void write() {
        auto data = Can::Data::build()->type(m_id)->value(to_vec())->build();
        auto request = Can::ServiceRequest::WriteDataByIdentifier::build()->data(data.value())->build();
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response = m_task->call(request.value());

        if(response->get_type() == Can::ServiceResponse::Type::Negative) {
            m_task->m_logger->error("Failed to write data");
        }
    }

    void read() {
        auto request = Can::ServiceRequest::ReadDataByIdentifier::build()->id(m_id)->build();
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response = m_task->call(request.value());
        if(response->get_type() == Can::ServiceResponse::Type::Negative) {
            m_task->m_logger->error("Failed to read data");
            return;
        }
        from_vec(
            std::static_pointer_cast<Can::ServiceResponse::ReadDataByIdentifier>(
                response)
                ->get_data()
                ->get_value());
    }

protected:

    virtual void create_fields() = 0;
    virtual void from_vec(std::vector<uint8_t> data) =0;
    virtual std::vector<uint8_t> to_vec() = 0;

    QSpinBox* create_hex_spin_box() {
        QSpinBox* box = new QSpinBox(this);
        box->setPrefix("0x");
        box->setDisplayIntegerBase(16);
        return box;
    }

    QHBoxLayout* m_layout;
    Can::DataIdentifier m_id;
    ConfigurationTask* m_task;
};

class StringField : public Field {
    Q_OBJECT
public:
    StringField(std::string name, Can::DataIdentifier id, int length) : Field(name, id), m_length(length) {
        
    }

protected:
    void create_fields() override {
        m_text = new QTextEdit(this);
        m_layout->addWidget(m_text);
        connect(m_text, &QTextEdit::textChanged, [=]() {
            QString text = m_text->toPlainText();
            if(text.length() > m_length) {
                text.truncate(m_length);
                m_text->setPlainText(text);
            }
        });
    }

    void from_vec(std::vector<uint8_t> data) override {
        std::string s = "";
        for(uint8_t c : data) {
            s += (char)c;
        }
        m_text->setPlainText(QString::fromStdString(s));
    }

    std::vector<uint8_t> to_vec() override {
        std::string s = m_text->toPlainText().toStdString();
        std::vector<uint8_t> res;
        for(char c : res) {
            res.push_back((uint8_t)c);
        }
        return res;
    }

private:
    QTextEdit* m_text;
    int m_length;
};

class IntField : public Field {
    Q_OBJECT
public:
    IntField(std::string name, Can::DataIdentifier id, int size) : Field(name, id), m_size(size) {}

protected:
    void create_fields() override {
        m_number = create_hex_spin_box();
        m_number->setMinimum(0);
        m_number->setMaximum((1 << m_size) - 1);
        m_layout->addWidget(m_number);
    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        uint64_t n = reader.read_int<uint64_t>(m_size).value();
        m_number->setValue(n);
    }

    std::vector<uint8_t> to_vec() override {
        Util::Writer writer(BYTES(m_size));
        writer.write_int<uint64_t>(m_number->value(), m_size);
        return writer.get_payload();
    }
    
private:
    QSpinBox* m_number;
    int m_size;
};

class VecField : public Field {
    Q_OBJECT
public:
    VecField(std::string name, Can::DataIdentifier id, int size) : Field(name, id), m_data(size, 0) {}

protected:
    void create_fields() override {
        QFrame* frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout(frame);
        for(int i = 0; i < m_data.size(); ++i) {
            QSpinBox* box = create_hex_spin_box();
            box->setMinimum(0);
            box->setMaximum(255);
            m_boxes.push_back(box);
            layout->addWidget(box);
        }
        m_layout->addWidget(frame);
    }

    void from_vec(std::vector<uint8_t> data) override {
        for(int i = 0; i < m_data.size(); ++i) {
            m_data[i] = data[i];
            m_boxes[i]->setValue(data[i]);
        }
    }

    std::vector<uint8_t> to_vec() override {
        return m_data;
    }

private:
    std::vector<uint8_t> m_data;
    std::vector<QSpinBox*> m_boxes;
};

struct MultiFieldItem {
    int size;
    std::string name;
};

class MultiField : public Field {
    Q_OBJECT
public:
    MultiField(std::string name, Can::DataIdentifier id, std::vector<MultiFieldItem> items) : Field(name, id), m_items(items) {}
protected:
    void create_fields() override {
        for(MultiFieldItem item : m_items) {
            QFrame* frame = new QFrame(this);
            QHBoxLayout* layout = new QHBoxLayout(frame);
            layout->addWidget(new QLabel(QString::fromStdString(item.name)));
            QSpinBox* box = create_hex_spin_box();
            box->setMinimum(0);
            box->setMaximum((1 << item.size) - 1);
            m_values.push_back(box);
            layout->addWidget(box);
            m_layout->addWidget(frame);
        }
    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        for(int i = 0; i < m_items.size(); ++i) {
            uint64_t n = reader.read_int<uint64_t>(m_items[i].size).value();
            m_values[i]->setValue(n);
        }
    }

    std::vector<uint8_t> to_vec() override {
        std::vector<uint8_t> res;
        for(int i = 0; i < m_items.size(); ++i) {
            uint64_t n = m_values[i]->value();
            Util::Writer writer(BYTES(m_items[i].size));
            writer.write_int<uint64_t>(n, m_items[i].size);
            for(uint8_t d : writer.get_payload()) {
                res.push_back(d);
            }
        }
        return res;
    }
    
private:
    std::vector<MultiFieldItem> m_items;
    std::vector<QSpinBox*> m_values;
};

class EnumField : public Field {
Q_OBJECT
public:
    EnumField(std::string name, Can::DataIdentifier id,
              std::vector<std::pair<std::string, uint64_t>> entries, int size)
        : Field(name, id), m_entries(entries), m_size(size) {}

    void create_fields() override {
        m_box = new QComboBox();
        for(auto [name, _] : m_entries) {
            m_box->addItem(QString::fromStdString(name));
        }
        m_layout->addWidget(m_box);
    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        int n = reader.read_int<uint64_t>(m_size).value();
        for(auto [name, m] : m_entries) {
            if(m == n) {
                m_box->setCurrentText(QString::fromStdString(name));
            }
        }
    }

    std::vector<uint8_t> to_vec() override {
        Util::Writer writer(BYTES(m_size));
        for (auto [name, n] : m_entries) {
            if (name == m_box->currentText().toStdString()) {
                writer.write_int<uint64_t>(n, m_size);
                return writer.get_payload();
            }
        }
    }

private:
    std::vector<std::pair<std::string, uint64_t>> m_entries;
    QComboBox* m_box;
    int m_size;
};
