#include "qlogger.h"

QLoggerWorker::QLoggerWorker(QObject* parent, QTextEdit* frame_log,
                             QTextEdit* message_log, QProgressBar* bar)
    : QObject(parent),
      m_frame_log(frame_log),
      m_message_log(message_log),
      m_timer(),
      m_mutex(),
      m_progress(bar) {
    m_timer.start();
}

void QLoggerWorker::received_frame(std::shared_ptr<Can::Frame> frame) {
    m_frame_log->setTextColor(QColor("blue"));
    std::vector<uint8_t> payload = frame->dump();
    QString payload_str = "ECU:    ";
    payload_str.append(vec_to_qstr(payload));
    m_frame_log->append(payload_str);
    m_frame_log->setTextColor(QColor("black"));
}

QString QLoggerWorker::vec_to_qstr(std::vector<uint8_t> vec) {
    QString str;
    for (uint8_t b : vec) {
        str.append(QString("%1").arg(b, 2, 16, QLatin1Char('0')));
        str.append(" ");
    }
    str.append("   ");
    str.append(QString("%1").arg(m_timer.elapsed(), 4, 10, QLatin1Char('0')));
    m_timer.restart();
    str = str.toUpper();
    str.append(" ms");
    return str;
}

void QLoggerWorker::transmitted_frame(std::shared_ptr<Can::Frame> frame) {
    std::vector<uint8_t> payload = frame->dump();
    QString payload_str = "Tester: ";
    payload_str.append(vec_to_qstr(payload));
    m_frame_log->append(payload_str);
}
void QLoggerWorker::received_service_response(
    std::shared_ptr<Can::ServiceResponse>) {}
void QLoggerWorker::transmitted_service_request(
    std::shared_ptr<Can::ServiceRequest>) {}

QString get_date_str() {
    return "[" +
           QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz") +
           "]";
}

void QLoggerWorker::info(std::string message) {
    m_message_log->setTextColor(QColor("gray"));
    m_message_log->append(get_date_str() +
                          "    INFO: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(info, message);
}
void QLoggerWorker::error(std::string message) {
    m_message_log->setTextColor(QColor("red"));
    m_message_log->append(get_date_str() +
                          "   ERROR: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(error, message);
}
void QLoggerWorker::warning(std::string message) {
    m_message_log->setTextColor(QColor("orange"));
    m_message_log->append(get_date_str() +
                          " WARNING: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(warning, message);
}
void QLoggerWorker::important(std::string message) {
    m_message_log->setStyleSheet("font-weight: bold;");
    m_message_log->append(get_date_str() +
                          "    INFO: " + QString::fromStdString(message));
    m_message_log->setStyleSheet("font-weight: regular;");
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(info, message);
}

void QLoggerWorker::progress(int a, bool err) {
    if (err) {
        QPalette p = m_progress->palette();
        p.setColor(QPalette::Highlight, Qt::red);
        m_progress->setPalette(p);
        return;
    }
    QPalette p = m_progress->palette();
    p.setColor(QPalette::Highlight, Qt::green);
    m_progress->setPalette(p);
    if (m_progress == nullptr) return;
    m_progress->setValue(a);
}
