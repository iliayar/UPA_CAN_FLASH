#include "can.h"
#include "service_all.h"
#include "frame_all.h"

#include <memory>
#include <QDateTime>

void QFlashTask::task() {
    Can::ServiceResponse* response;
    response =
        call(Can::ServiceRequest_DiagnosticSessionControl::build()
                 ->subfunction(Can::DiagnosticSessionControl_SubfunctionType::
                                   extendDiagnosticSession)
                 ->build());
}

QLogger::QLogger(QObject* parent, QTextEdit* frame_log, QTextEdit* message_log)
    : QObject(parent), m_frame_log(frame_log), m_message_log(message_log), m_timer() {

    m_timer.start();
    
    received_frame(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x01, 0x02, 0x03})));
    transmitted_frame(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x01, 0xa2, 0x03})));
    received_frame(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x01, 0x02, 0x03})));
    transmitted_frame(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x01, 0x02, 0x03})));
    error("Test error");
    info("Test info");
    warning("Test warning");
}

void QLogger::received_frame(std::shared_ptr<Can::Frame> frame)
{
    m_frame_log->setTextColor(QColor("blue"));
    std::vector<uint8_t> payload = frame->dump();
    QString payload_str ="ECU:    ";
    payload_str.append(vec_to_qstr(payload));
    m_frame_log->append(payload_str);
    m_frame_log->setTextColor(QColor("black"));
}

QString QLogger::vec_to_qstr(std::vector<uint8_t> vec) {
    QString str;
    for(uint8_t b : vec) {
        str.append(QString("%1").arg(b, 2, 16, QLatin1Char('0')));
        str.append(" ");
    }
    str.append("   ");
    str.append(QString("%1").arg(m_timer.elapsed(), 3, 10, QLatin1Char('0')));
    m_timer.restart();
    str = str.toUpper();
    str.append(" ms");
    return str;
}

void QLogger::transmitted_frame(std::shared_ptr<Can::Frame> frame)
{
    std::vector<uint8_t> payload = frame->dump();
    QString payload_str ="Tester: ";
    payload_str.append(vec_to_qstr(payload));
    m_frame_log->append(payload_str);
}
void QLogger::received_service_response(Can::ServiceResponse*)
{}
void QLogger::transmitted_service_request(Can::ServiceRequest*)
{}

QString get_date_str() {
    return "[" + QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss,zzz") + "]";
}

void QLogger::info(std::string message)
{
    m_message_log->setTextColor(QColor("gray"));
    m_message_log->append(get_date_str() + "    INFO: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
}
void QLogger::error(std::string message)
{
    m_message_log->setTextColor(QColor("red"));
    m_message_log->append(get_date_str() + "   ERROR: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
}
void QLogger::warning(std::string message)
{
    m_message_log->setTextColor(QColor("orange"));
    m_message_log->append(get_date_str() + " WARNING: " + QString::fromStdString(message));
    m_message_log->setTextColor(QColor("black"));
}
