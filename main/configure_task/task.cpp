#include "task.h"

#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QVBoxLayout>

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
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), window);
        QVBoxLayout* layout = new QVBoxLayout(group);
        m_groups[name] = group;
        main_layout->addWidget(group);
        group->hide();
        for (Field* field : fields) {
            field->init(this);
            field->setParent(group);
            layout->addWidget(field);
        }
    }

    connect(groups_list, &QListWidget::currentItemChanged,
            [this](QListWidgetItem* item, QListWidgetItem* prev) {
                if (prev != nullptr) {
                    auto prev_group = m_groups[prev->text().toStdString()];
                    if (prev_group != nullptr && prev_group->isVisible())
                        prev_group->hide();
                }
                if (item != nullptr) {
                    auto cur_group = m_groups[item->text().toStdString()];
                    if (cur_group != nullptr && !cur_group->isVisible())
                        cur_group->show();
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
