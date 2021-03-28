#include "task.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QDialog>
#include <QGroupBox>
#include "fields.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger)
    : QTask(logger)
{
    m_logger->info("Hello World!"); 

    QDialog* main_dialog = new QDialog();

    QHBoxLayout* main_layout = new QHBoxLayout(main_dialog);

    QListWidget* groups_list = new QListWidget(main_dialog);

    main_layout->addWidget(groups_list);

    for(int i = 0; i < GROUPS.size(); ++i) {
        QListWidgetItem* item =
            new QListWidgetItem(QString::fromStdString(GROUPS[i]), groups_list);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), main_dialog);
        QVBoxLayout* layout = new QVBoxLayout(group);
        m_groups[GROUPS[i]] = group;
        main_layout->addWidget(group);
        group->hide();
        Field* field;
        // if(i == 0) {
        //     field = new StringField("String Test", 10);
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        // } else if(i == 1) {
        //     field = new IntField("Int Test", 16);
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        //     field = new StringField("String Test", 10);
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        //     field = new VecField("Vec Test", 8);
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        // } else if(i == 2) {
        //     field = new VecField("Vec Test", 8);
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        // } else if(i == 3) {
        //     field = new MultiField("Mutli Test", {{8, "Value 8"}, {16, "Value 16"}});
        //     field->init();
        //     field->setParent(group);
        //     layout->addWidget(field);
        // }
    }

    main_dialog->setLayout(main_layout);

    connect(groups_list, &QListWidget::currentItemChanged,
            [&](QListWidgetItem* item, QListWidgetItem* prev) {
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

    m_main_dialog = main_dialog;
}

void ConfigurationTask::task() {

    m_main_dialog->exec();

}
