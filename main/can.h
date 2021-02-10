#pragma once

#include <QObject>
#include <QTextEdit>
#include <QElapsedTimer>
#include <memory>

#include "communicator.h"
#include "frame.h"
#include "service.h"
#include "task.h"

class QFlashTask : public QObject, public Can::AsyncTask {
    Q_OBJECT
public:
    QFlashTask(QObject* parent) : QObject(parent) {}

    void task();
};

class QLogger : public QObject, public Can::Logger {
    Q_OBJECT
public:
    QLogger(QObject*, QTextEdit*, QTextEdit*);

    void received_frame(std::shared_ptr<Can::Frame>);
    void transmitted_frame(std::shared_ptr<Can::Frame>);
    void received_service_response(Can::ServiceResponse*);
    void transmitted_service_request(Can::ServiceRequest*);

    void error(std::string);
    void info(std::string);
    void warning(std::string);
    
private:
    QString vec_to_qstr(std::vector<uint8_t>);

    QElapsedTimer m_timer;
    
    QTextEdit* m_frame_log;
    QTextEdit* m_message_log;

    std::mutex m_mutex;
};
