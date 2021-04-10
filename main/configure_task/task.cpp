#include "task.h"

#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QScrollArea>

#include "config.h"
#include "fields.h"
#include "service_all.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger)
    : QTask(logger) {
    QWidget* window = new QWidget(nullptr);
    QHBoxLayout* main_layout = new QHBoxLayout(window);


    QFrame* left_frame = new QFrame(window);
    left_frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    QVBoxLayout* left_layout = new QVBoxLayout(left_frame);
    QListWidget* groups_list = new QListWidget(left_frame);

    QPushButton* err_btn = new QPushButton("Read errors", window);

    groups_list->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    main_layout->addWidget(left_frame);
    left_layout->addWidget(groups_list);
    left_layout->addWidget(err_btn);

    for (auto& [name, fields] : m_config.fields) {
        QListWidgetItem* item =
            new QListWidgetItem(QString::fromStdString(name), groups_list);
        QScrollArea* scroll = new QScrollArea(window);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), scroll);
        scroll->horizontalScrollBar()->setDisabled(true);
        scroll->setWidget(group);
        scroll->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QVBoxLayout* layout = new QVBoxLayout(group);
        m_groups[name] = {scroll, group};
        group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        main_layout->addWidget(scroll);
        scroll->hide();
        for (Field* field : fields) {
            field->init(this);
            field->setParent(group);
            field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            // field->setMinimumSize(50, 200);
            // field->adjustSize();
            layout->addWidget(field);
        }
        // scroll->adjustSize();
        // groups_list->adjustSize();
        group->adjustSize();
    }

    connect(groups_list, &QListWidget::currentItemChanged,
            [this](QListWidgetItem* item, QListWidgetItem* prev) {
                if (prev != nullptr) {
                    auto prev_group = m_groups[prev->text().toStdString()];
                    if (prev_group.first != nullptr && prev_group.first->isVisible())
                        prev_group.first->hide();
                }
                if (item != nullptr) {
                    auto cur_group = m_groups[item->text().toStdString()];
                    if (cur_group.first != nullptr && !cur_group.first->isVisible()) {
                        cur_group.first->show();
                        // cur_group.first->adjustSize();
                        // cur_group.second->adjustSize();
                    }
                }
            });

    connect(err_btn, &QPushButton::clicked, this, &ConfigurationTask::read_errors);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->setFocus();
    m_window = window;
    groups_list->setCurrentRow(0);
}

void ConfigurationTask::task() {

    auto response = call(Can::ServiceRequest::DiagnosticSessionControl::build()
                        ->subfunction(Can::ServiceRequest::DiagnosticSessionControl::
                                          Subfunction::extendDiagnosticSession)
                        ->build()
                        .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot enter extendDiagnosticSession");
        return;
    }
    QEventLoop loop;
    connect(m_window, &QWidget::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}

void ConfigurationTask::read_errors() {
    auto response =
        call(Can::ServiceRequest::ReadDTCInformation::build()
                 ->subfunction(Can::ServiceRequest::ReadDTCInformation::
                                   Subfunction::reportDTCByStatusMask)
                 ->mask(Can::confirmedDTC | Can::testFailedDTC)
                 ->build()
                 .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to read errors");
        return;
    }
    for(auto err : std::static_pointer_cast<Can::ServiceResponse::ReadDTCInformation>(response)->get_records()) {
        auto it1 = m_config.errors.find(err->get_type());
        if(it1 == m_config.errors.end()) {
            continue;
        }
        auto& [name, var_map] = it1->second;
        auto it2 = var_map.find(err->get_status());
        if(it2 == var_map.end()) {
            continue;
        }
        auto& [mnemonic, description] = it2->second;
        m_logger->error(name + " " + mnemonic + " " + description);
    }
}
