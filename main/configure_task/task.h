#pragma once

#include "../qtask.h"

#include <QLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QMainWindow>
#include <QCloseEvent>
#include <QSettings>
#include <QEventLoop>
#include <unordered_map>
#include "config.h"

class ConfigurationWindow : public QMainWindow {
Q_OBJECT
signals:
    void closed();
private:
    void closeEvent(QCloseEvent* e) {
        emit closed();
        e->accept();
    }
};

class ConfigurationTask : public QTask {

public:

    ConfigurationTask(std::shared_ptr<QLogger> logger, bool security);

    void task() override;

    friend class Field;

private:
    void read_errors(uint8_t);
    void clear_errors();
    void factory_reset();

    // std::unordered_map<std::string, std::pair<QWidget*, QWidget*>> m_groups;
    ConfigurationWindow* m_window;
    QTextEdit* m_err_log;
    DataConfig m_config{};
    QSettings m_settings;
    QWidget* m_main_widget;

    bool m_security;
};
