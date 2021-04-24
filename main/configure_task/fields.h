#pragma once

#include <QComboBox>
#include <QDialog>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>

#include "service_all.h"
#include "task.h"

struct SpinBox {
    QFrame* frame;
    QSpinBox* box;
    QLabel* prefix;
};

class Field : public QGroupBox {
    Q_OBJECT
public:
    Field(QWidget* parent, std::string name, uint16_t id)
        : QGroupBox(QString::fromStdString(name), parent), m_id(id) {}
    Field(std::string name, uint16_t id)
        : Field(nullptr, name, id) {}

    void init(ConfigurationTask* task) {
        m_task = task;
        QHBoxLayout* main_layout = new QHBoxLayout(this);
        QFrame* inner_frame = new QFrame();
        m_layout = new QHBoxLayout(inner_frame);
        create_fields();
        main_layout->addWidget(inner_frame);
        main_layout->setMargin(0);
        m_layout->setMargin(0);
        QFrame* frame = new QFrame(this);
        QHBoxLayout* layout = new QHBoxLayout(frame);
        QPushButton* write_btn = new QPushButton(tr("&Write"));
        QPushButton* read_btn = new QPushButton(tr("&Read"));
        layout->addWidget(write_btn);
        layout->addWidget(read_btn);
        frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        main_layout->addWidget(frame);
        connect(read_btn, &QPushButton::pressed, this, &Field::read);
        connect(write_btn, &QPushButton::pressed, this, &Field::write);
    }
public slots:

    void diagnostic_session() {
        auto response = m_task->call(
            Can::ServiceRequest::DiagnosticSessionControl::build()
                ->subfunction(Can::ServiceRequest::DiagnosticSessionControl::
                                  Subfunction::extendDiagnosticSession)
                ->build()
                .value());

        IF_NEGATIVE(response) {
            m_task->m_logger->error("Failed ot enter extendDiagnosticSession");
            return;
        }
    }

    void write() {
        diagnostic_session();
        auto data = Can::Data::build()->raw_type(m_id)->value(to_vec())->build();
        auto request = Can::ServiceRequest::WriteDataByIdentifier::build()
                           ->data(data.value())
                           ->build();
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            m_task->call(request.value());

        if (response->get_type() == Can::ServiceResponse::Type::Negative) {
            m_task->m_logger->error("Failed to write data");
        }
    }

    void read() {
        diagnostic_session();
        auto request = Can::ServiceRequest::ReadDataByIdentifier::build()
                           ->raw_id(m_id)
                           ->build();
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            m_task->call(request.value());
        if (response->get_type() == Can::ServiceResponse::Type::Negative) {
            m_task->m_logger->error("Failed to read data");
            return;
        }
        from_vec(std::static_pointer_cast<
                     Can::ServiceResponse::ReadDataByIdentifier>(response)
                     ->get_data()
                     ->get_value());
    }

protected:
    virtual void create_fields() = 0;
    virtual void from_vec(std::vector<uint8_t> data) = 0;
    virtual std::vector<uint8_t> to_vec() = 0;

    SpinBox create_hex_spin_box() {
        QFrame* frame = new QFrame(this);
        frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        frame->setContentsMargins(0, 0, 0, 0);
        QHBoxLayout* layout = new QHBoxLayout(frame);
        layout->setSpacing(0);
        layout->setMargin(0);
        QLabel* label = new QLabel("0x");
        label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        label->setFixedWidth(20);
        QSpinBox* box = new QSpinBox(frame);
        box->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        layout->addWidget(label);
        layout->addWidget(box);
        QFont font = box->font();
        font.setCapitalization(QFont::AllUppercase);
        box->setFont(font);
        box->setDisplayIntegerBase(16);
        box->setButtonSymbols(QAbstractSpinBox::NoButtons);
        return {frame, box, label};
    }

    void toggle_hex_dec(std::vector<SpinBox> boxes, int state) {
        for (auto box : boxes) {
            if (state != 0) {
                box.box->setDisplayIntegerBase(10);
                box.prefix->setText("");
            } else {
                box.box->setDisplayIntegerBase(16);
                box.prefix->setText("0x");
            }
        }
    }

    void log(std::string s) {
    m_task->m_logger->info(s);
    }

    QHBoxLayout* m_layout;
    uint16_t m_id;
    ConfigurationTask* m_task;
};

class StringField : public Field {
    Q_OBJECT
public:
    StringField(std::string name, uint16_t id, int length)
        : Field(name, id), m_length(BYTES(length)) {}

protected:
    void create_fields() override {
        m_text = new QLineEdit(this);
        m_layout->addWidget(m_text);
        m_text->setMaxLength(m_length);
    }

    void from_vec(std::vector<uint8_t> data) override {
        std::string s = "";
        for (uint8_t c : data) {
            s += (char)c;
        }
        m_text->setText(QString::fromStdString(s));
    }

    std::vector<uint8_t> to_vec() override {
        std::string s = m_text->text().toStdString();
        std::vector<uint8_t> res(m_length, 0);
        for (int i = 0; i < m_length; ++i) {
            if(i < s.length()) {
		res[i] = static_cast<uint8_t>(s[i]);
	    } else {
		res[i] = 0x00;
	    }
        }
        return res;
    }

private:
    QLineEdit* m_text;
    int m_length;
};

class IntField : public Field {
    Q_OBJECT
public:
    IntField(std::string name, uint16_t id, int size)
        : Field(name, id), m_size(size) {}

protected:
    void create_fields() override {
        m_number = create_hex_spin_box();
        m_number.box->setMinimum(0);
        m_number.box->setMaximum((1 << m_size) - 1);
        m_layout->addWidget(m_number.frame);

    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        uint64_t n = reader.read_int<uint64_t>(m_size).value();
        m_number.box->setValue(n);
    }

    std::vector<uint8_t> to_vec() override {
        Util::Writer writer(BYTES(m_size));
        writer.write_int<uint64_t>(m_number.box->value(), m_size);
        return writer.get_payload();
    }

private:
    SpinBox m_number;
    int m_size;
};

class VecField : public Field {
    Q_OBJECT
public:
    VecField(std::string name, uint16_t id, int size)
        : Field(name, id), m_size(BYTES(size)) {}

protected:
    void create_fields() override {
        QFrame* frame = new QFrame(this);
        frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        QHBoxLayout* layout = new QHBoxLayout(frame);
        QCheckBox* checkbox = new QCheckBox("decimal", frame);
        QPushButton* copy_btn = new QPushButton("Copy", frame);
        layout->addWidget(copy_btn);
        layout->addWidget(checkbox);
        std::vector<std::string> top_labels = {"G", "0", "21", "25", "42", "60", "76", "115", "145", "180", "213", "250", "280", "350", "420"};
        std::vector<std::string> bot_labels = {"", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"};
        for (int i = 0; i < m_size; ++i) {
            QFrame* box_frame = new QFrame(frame);
            box_frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            QVBoxLayout* box_layout = new QVBoxLayout(box_frame);
            box_layout->setSpacing(0);
            box_layout->setMargin(0);
            SpinBox box = create_hex_spin_box();
            box.box->setMinimum(0);
            box.box->setMaximum(255);
            m_boxes.push_back(box);
            QLabel* top_label;
            QLabel* bot_label;
            if(i < top_labels.size()) {
                top_label = new QLabel(QString::fromStdString(top_labels[i]));
            } else {
                top_label = new QLabel("");
            }
            if(i < bot_labels.size()) {
                bot_label = new QLabel(QString::fromStdString(bot_labels[i]));
            } else {
                bot_label = new QLabel("");
            }
            top_label->setStyleSheet("color: red");
            bot_label->setStyleSheet("color: red");
            top_label->setAlignment(Qt::AlignRight);
            bot_label->setAlignment(Qt::AlignRight);
            box_layout->addWidget(top_label);
            box_layout->addWidget(box.frame);
            box_layout->addWidget(bot_label);
            layout->addWidget(box_frame);
        }
        m_layout->addWidget(frame);
        connect(checkbox, &QCheckBox::stateChanged, [=](int state) {
            this->toggle_hex_dec(this->m_boxes, state);
        });
        connect(copy_btn, &QPushButton::clicked, [=]() {
            QString res;
            int i = 0;
            bool dec = checkbox->isChecked();
            for(auto box : this->m_boxes) {
                if(dec) {
                    if(i > 0) {
                        res += ",";
                    }
                    res += QString("%1").arg(box.box->value(), 0, 10);
                } else {
                    res += QString("%1").arg(box.box->value(), 2, 16,
                                             QLatin1Char('0'));
                }
                i++;
            }
            // res = QString("0x") + res;
            QClipboard* cb = QApplication::clipboard();
            cb->setText(res, QClipboard::Clipboard);
            if(cb->supportsSelection()) {
                cb->setText(res, QClipboard::Selection);
            }
            // qDebug() << res;
        });
    }

    void from_vec(std::vector<uint8_t> data) override {
        for (int i = 0; i < m_size; ++i) {
            m_boxes[i].box->setValue(data[i]);
        }
    }

    std::vector<uint8_t> to_vec() override {
        std::vector<uint8_t> res{};
        for(int i = 0; i < m_size; ++i){
            res.push_back(static_cast<uint8_t>(m_boxes[i].box->value()));
        }
        return res;
    }

private:
    int m_size;
    std::vector<SpinBox> m_boxes;
};

struct MultiFieldItem {
    int size;
    std::string name;
};

class MultiField : public Field {
    Q_OBJECT
public:
    MultiField(std::string name, uint16_t id,
               std::vector<MultiFieldItem> items)
        : Field(name, id), m_items(items) {}

protected:
    void create_fields() override {
        QCheckBox* checkbox = new QCheckBox("decimal", this);
        m_layout->addWidget(checkbox);
        m_layout->setAlignment(Qt::AlignLeft);
        for (MultiFieldItem item : m_items) {
            QFrame* frame = new QFrame(this);
            frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            QHBoxLayout* layout = new QHBoxLayout(frame);
            layout->addWidget(new QLabel(QString::fromStdString(item.name) + ": "));
            SpinBox box = create_hex_spin_box();
            box.box->setMinimum(0);
            box.box->setMaximum((1 << item.size) - 1);
            m_values.push_back(box);
            layout->addWidget(box.frame);
            m_layout->addWidget(frame);
        }
        connect(checkbox, &QCheckBox::stateChanged, [=](int state) {
            this->toggle_hex_dec(this->m_values, state);
        });
    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        for (int i = 0; i < m_items.size(); ++i) {
            uint64_t n = reader.read_int<uint64_t>(m_items[i].size).value();
            m_values[i].box->setValue(n);
        }
    }

    std::vector<uint8_t> to_vec() override {
        std::vector<uint8_t> res;
        for (int i = 0; i < m_items.size(); ++i) {
            uint64_t n = m_values[i].box->value();
            Util::Writer writer(BYTES(m_items[i].size));
            writer.write_int<uint64_t>(n, m_items[i].size);
            for (uint8_t d : writer.get_payload()) {
                res.push_back(d);
            }
        }
        return res;
    }

private:
    std::vector<MultiFieldItem> m_items;
    std::vector<SpinBox> m_values;
};

class EnumField : public Field {
    Q_OBJECT
public:
    EnumField(std::string name, uint16_t id,
              std::vector<std::pair<std::string, uint64_t>> entries, int size)
        : Field(name, id), m_entries(entries), m_size(size) {}

    void create_fields() override {
        m_box = new QComboBox();
        for (auto [name, _] : m_entries) {
            m_box->addItem(QString::fromStdString(name));
        }
        m_layout->addWidget(m_box);
    }

    void from_vec(std::vector<uint8_t> data) override {
        Util::Reader reader(data);
        int n = reader.read_int<uint64_t>(m_size).value();
        for (auto [name, m] : m_entries) {
            if (m == n) {
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
        return writer.get_payload();
    }

private:
    std::vector<std::pair<std::string, uint64_t>> m_entries;
    QComboBox* m_box;
    int m_size;
};
