#include "task.h"


#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QDialog>
#include <QGroupBox>
#include "fields.h"
#include "config.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger, QWidget* parent)
    : QTask(logger), m_parent(parent)
{
	QWidget* window = new QWidget(nullptr);
    QHBoxLayout* main_layout = new QHBoxLayout(window);

    QListWidget* groups_list = new QListWidget(window);

    main_layout->addWidget(groups_list);

    DataConfig config{};

    for(auto& [name, fields] : config.fields) {
        QListWidgetItem* item =
            new QListWidgetItem(QString::fromStdString(name), groups_list);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), window);
        QVBoxLayout* layout = new QVBoxLayout(group);
        m_groups[name] = group;
        main_layout->addWidget(group);
        group->hide();
        for(Field* field : fields) {
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

    	window->setAttribute( Qt::WA_DeleteOnClose );
	window->show();
	window->setFocus();
	m_window = window;

}

void ConfigurationTask::task() {
	QEventLoop loop;
	connect(m_window, &QWidget::destroyed, &loop, &QEventLoop::quit);
	loop.exec();
}
