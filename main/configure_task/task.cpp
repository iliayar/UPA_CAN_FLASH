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

    QListWidget* groups_list = new QListWidget(window);

    main_layout->addWidget(groups_list);

    DataConfig config{};

    for (auto& [name, fields] : config.fields) {
        QListWidgetItem* item =
            new QListWidgetItem(QString::fromStdString(name), groups_list);
        QScrollArea* scroll = new QScrollArea(window);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), scroll);
        scroll->setWidget(group);
        QVBoxLayout* layout = new QVBoxLayout(group);
        m_groups[name] = {scroll, group};
        main_layout->addWidget(scroll);
        scroll->hide();
        for (Field* field : fields) {
            field->init(this);
            field->setParent(group);
            field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            field->setMinimumSize(50, 200);
            layout->addWidget(field);
        }
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
                        cur_group.first->adjustSize();
                        cur_group.second->adjustSize();
                    }
                }
            });

    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->setFocus();
    m_window = window;
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
