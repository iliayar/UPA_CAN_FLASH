#include <QObject>
#include <QDebug>

#include "qcommunicator.h"
#include "qtask.h"

class TestQLoggerWorker : public QLoggerWorker {
    Q_OBJECT
public:
    TestQLoggerWorker(QObject* parent = nullptr) : QLoggerWorker(parent, nullptr, nullptr) {}

public slots:

    void received_frame(std::shared_ptr<Can::Frame::Frame> frame) override {
        qDebug() << "Frame recievied";
        m_received_frames.push_back(frame);
    }
    void transmitted_frame(std::shared_ptr<Can::Frame::Frame> frame) override {
        qDebug() << "Frame transmitted";
        m_transmitted_frames.push_back(frame);
    }
    void received_service_response(
        std::shared_ptr<Can::ServiceResponse::ServiceResponse>) override {}
    void transmitted_service_request(
        std::shared_ptr<Can::ServiceRequest::ServiceRequest>) override {}
    void error(std::string const&) override { m_errors++; }
    void info(std::string const&) override { m_infos++; }
    void warning(std::string const&) override { m_warnings++; }
    void important(std::string const&) override { m_imporants++; }
    void progress(int, bool err) override {}

public:
    int m_errors = 0;
    int m_infos = 0;
    int m_warnings = 0;
    int m_imporants = 0;

    std::vector<std::shared_ptr<Can::Frame::Frame>> m_received_frames;
    std::vector<std::shared_ptr<Can::Frame::Frame>> m_transmitted_frames;
};

class TestTask : public QTask {
    Q_OBJECT
public:

    TestTask(std::shared_ptr<QLogger> logger) : QTask(logger) {}
    
    void task() {
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response;
        response = call(Can::ServiceRequest::ReadDataByIdentifier::build()
                            ->id(Can::DataIdentifier::VIN)
                            ->build()
                            .value());
    }
};

class TestWindow : public QObject {
    Q_OBJECT
public:
    TestWindow(std::shared_ptr<QLogger> logger, QObject* parent = nullptr) : QObject(parent) {
        QCommunicator* m_communicator = new QCommunicator(logger);
        connect(m_communicator, &QCommunicator::fetch_frame, this,
                &TestWindow::check_frames_to_write);
        connect(this, &TestWindow::frame_received, m_communicator,
                &QCommunicator::push_frame);
        connect(m_communicator, &QCommunicator::task_exited, this,
                &TestWindow::task_done);
        connect(this, &TestWindow::set_task, m_communicator,
                &QCommunicator::set_task);
        m_communicator->moveToThread(&m_communicator_thread);
        m_communicator_thread.start();
    }

    void test(std::shared_ptr<QTask> task, std::vector<std::shared_ptr<Can::Frame::Frame>> frames,
              std::vector<int> delays) {
        emit set_task(task);
        for (int i = 0; i < frames.size(); ++i) {
            qDebug() << "AYAYAY";
            emit frame_received(frames[i]);
            if (i < delays.size()) {
                QThread::usleep(delays[i]);
            }
        }
    }

    ~TestWindow() {
        m_communicator_thread.terminate();
        m_communicator_thread.wait();
    }
public slots:
    void check_frames_to_write(std::shared_ptr<Can::Frame::Frame> frame) {}
    void task_done() {
    }
signals:
    void frame_received(std::shared_ptr<Can::Frame::Frame>);
    void set_task(std::shared_ptr<QTask>);
private:
    QThread m_communicator_thread;
};
