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

void QLoggerWorker::received_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    auto maybe_payload = frame->dump();
    if(!maybe_payload) {
        error("Failed to dump frame provided to logger");
        return;
    }
    std::vector<uint8_t> payload = maybe_payload.value();
    m_frame_log->setTextColor(QColor("blue"));
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

void QLoggerWorker::transmitted_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    auto maybe_payload = frame->dump();
    if(!maybe_payload) {
        error("Failed to dump frame provided to logger");
        return;
    }
    std::vector<uint8_t> payload = maybe_payload.value();
    QString payload_str = "Tester: ";
    payload_str.append(vec_to_qstr(payload));
    m_frame_log->append(payload_str);
}
void QLoggerWorker::received_service_response(
    std::shared_ptr<Can::ServiceResponse::ServiceResponse>) {}
void QLoggerWorker::transmitted_service_request(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest>) {}

QString get_date_str() {
    return "[" +
           QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz") +
           "]";
}

void QLoggerWorker::info(std::string const& message) {
    m_message_log->setTextColor(QColor("gray"));
    m_message_log->append(get_date_str() +
                          "    INFO: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(info, message);
}
void QLoggerWorker::error(std::string const& message) {
    m_message_log->setTextColor(QColor("red"));
    m_message_log->append(get_date_str() +
                          "   ERROR: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(error, message);
}
void QLoggerWorker::warning(std::string const& message) {
    m_message_log->setTextColor(QColor("orange"));
    m_message_log->append(get_date_str() +
                          " WARNING: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
    m_message_log->verticalScrollBar()->setValue(
        m_message_log->verticalScrollBar()->maximum());
    DEBUG(warning, message);
}
void QLoggerWorker::important(std::string const& message) {
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

QLoggerWorker* QLogger::get_worker() { return m_worker; }
void QLogger::received_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    emit signal_received_frame(frame);
}
void QLogger::transmitted_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    emit signal_transmitted_frame(frame);
}
void QLogger::received_service_response(
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> response) {
    emit signal_received_service_response(response);
}
void QLogger::transmitted_service_request(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> request) {
    emit signal_transmitted_service_request(request);
}

void QLogger::error(std::string const& s) { emit signal_error(s); }
void QLogger::info(std::string const& s) { emit signal_info(s); }
void QLogger::warning(std::string const& s) { emit signal_warning(s); }
void QLogger::important(std::string const& s) { emit signal_important(s); }
void QLogger::progress(int a, bool err) { emit signal_progress(a, err); }
