#include "qcommunicator.h"

#include <QObject>
#include <QThread>
#include <memory>
#include <stdexcept>

#include "frame_all.h"
#include "service.h"
#include "task.h"

void QCommunicator::set_task(std::shared_ptr<QTask> task) {
    DEBUG(info, "QCommunicator setting task");

    worker_done();

    if (m_task != nullptr) {
        if (m_task->isRunning()) {
            m_task->terminate();
        }
        m_task->wait();
        disconnect(this, &QCommunicator::response, m_task.get(),
                   &QTask::response);
        disconnect(m_task.get(), &QTask::request, this,
                   &QCommunicator::request);
        disconnect(m_task.get(), &QTask::finished, this,
                   &QCommunicator::task_done);
        worker_done();
        emit task_exited();
    }

    m_task = task;
    if (task == nullptr) return;

    connect(this, &QCommunicator::response, task.get(), &QTask::response);
    connect(task.get(), &QTask::request, this, &QCommunicator::request);
    connect(task.get(), &QTask::finished, this, &QCommunicator::task_done);
    DEBUG(info, "starting task");

    m_task->start();
}

void QCommunicator::task_done() {
    DEBUG(info, "task done");
    m_task->wait();
    DEBUG(info, "task exited");
    m_logger->info("Task exited");
    set_task(nullptr);
}

void QCommunicator::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    if (frame == nullptr) return;

    emit response(nullptr, 1);
    m_logger->received_frame(frame);

    DEBUG(info, "QCommunicator push_frame");
    if (m_worker != nullptr) {
        emit push_frame_worker(frame);
        return;
    }
    m_worker = std::make_unique<QReceiver>(this);

    emit operate_receiver(frame);
    DEBUG(info, "QReceiver created");
}

void QCommunicator::fetch_frame_worker(std::shared_ptr<Can::Frame::Frame> frame) {
    if (frame == nullptr) return;

    emit response(nullptr, 1);
    DEBUG(info, "QCommunicator fetch_frame_worker");

    emit fetch_frame(frame);
    m_logger->transmitted_frame(frame);
}

void QCommunicator::request(std::shared_ptr<Can::ServiceRequest::ServiceRequest> r) {
    if (r == nullptr) return;
    if (m_worker != nullptr) {
        m_logger->warning("Restarting worker");
        worker_done();
    }

    DEBUG(info, "QCommunicator receive request");
    m_worker = std::make_unique<QTransmitter>(this);

    emit operate_transmitter(r);
    DEBUG(info, "QTransmitter created");
}

void QCommunicator::worker_error(WorkerError e) {
    switch (e) {
        case WorkerError::Timeout:
            m_logger->warning("Frame worker timed out");
            break;
        case WorkerError::Other:
            m_logger->error("Frame worker error");
            break;
    }
    worker_done();
}
void QCommunicator::worker_done() {
    DEBUG(info, "QCommunicator worker_done");
    if (m_worker == nullptr) return;
    auto resp = m_worker->try_get_response();
    if(resp) {
        emit response(resp.value());
    }
    m_worker = nullptr;
    DEBUG(info, "QCommunicator worker_done quit");
}

QCommunicator::~QCommunicator() {
    m_worker_thread.terminate();
    m_worker_thread.wait();
}

QWorker::QWorker(QCommunicator const* communicator)
    : m_communicator(communicator) {}

QTransmitter::QTransmitter(QCommunicator const* communicator)
    : QWorker(communicator) {
    connect(m_communicator, &QCommunicator::push_frame_worker, this,
            &QTransmitter::push_frame);
    connect(this, &QTransmitter::fetch_frame, m_communicator,
            &QCommunicator::fetch_frame_worker);
    connect(this, &QTransmitter::worker_done, m_communicator,
            &QCommunicator::worker_done);
    connect(this, &QTransmitter::worker_error, m_communicator,
            &QCommunicator::worker_error);
    connect(m_communicator, &QCommunicator::operate_transmitter, this,
            &QTransmitter::init);
}

QTransmitter::~QTransmitter() {
    disconnect(this, &QTransmitter::fetch_frame, m_communicator,
               &QCommunicator::fetch_frame_worker);
    disconnect(m_communicator, &QCommunicator::push_frame_worker, this,
               &QTransmitter::push_frame);
    disconnect(this, &QTransmitter::worker_done, m_communicator,
               &QCommunicator::worker_done);
    disconnect(this, &QTransmitter::worker_error, m_communicator,
               &QCommunicator::worker_error);
    disconnect(m_communicator, &QCommunicator::operate_transmitter, this,
               &QTransmitter::init);
}

void QTransmitter::init(std::shared_ptr<Can::ServiceRequest::ServiceRequest> request) {
    init_timer();
    auto frames = Can::service_to_frames(request);
    if(!frames || frames->size() == 0) {
        emit worker_error(WorkerError::Other);
        return;
    }
    m_frames = frames.value();
    switch (m_frames[0]->get_type()) {
        case Can::Frame::Type::SingleFrame: {
            DEBUG(info, "QTransmitter fetch single frame");
            update_timer();
            emit fetch_frame(m_frames[0]);
            emit worker_done();
            break;
        }
        case Can::Frame::Type::FirstFrame: {
            DEBUG(info, "transmitter first frame");
            m_i = 1;
            update_timer();
            emit fetch_frame(m_frames[0]);
            break;
        }
        default:
            DEBUG(info, "transmitter error frame");
            emit worker_error(WorkerError::Other);
            break;
    }
}

void QTransmitter::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    DEBUG(info, "QTransmitter push_frame");
    if (frame->get_type() == Can::Frame::Type::FlowControl) {
        DEBUG(info, "transmitter pushing with m_wait_fc and flow control frame")
        Can::Frame::FlowStatus status =
            std::static_pointer_cast<Can::Frame::FlowControl>(frame)->get_status();
        if (status ==
            Can::Frame::FlowStatus::WaitForAnotherFlowControlMessageBeforeContinuing) {
            return;
        } else if (status == Can::Frame::FlowStatus::OverflowAbortTransmission) {
            emit worker_error(WorkerError::Other);
            return;
        }
        int fc_block_size =
            std::static_pointer_cast<Can::Frame::FlowControl>(frame)->get_block_size();
        if (fc_block_size == 0) fc_block_size = m_frames.size();
        int fc_min_time = std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                              ->get_min_separation_time();
        for (int i = 0; i < fc_block_size && m_i < m_frames.size();
             ++i, ++m_i) {
            QThread::msleep(fc_min_time);
            update_timer();
            emit fetch_frame(m_frames[m_i]);
        }
        if (m_i >= m_frames.size()) emit worker_done();
    }
}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
QTransmitter::try_get_response() {
    return {};
}

QReceiver::QReceiver(QCommunicator const* communicator)
    : QWorker(communicator) {
    connect(this, &QReceiver::fetch_frame, m_communicator,
            &QCommunicator::fetch_frame_worker);
    connect(m_communicator, &QCommunicator::push_frame_worker, this,
            &QReceiver::push_frame);
    connect(this, &QReceiver::worker_done, m_communicator,
            &QCommunicator::worker_done);
    connect(this, &QReceiver::worker_error, m_communicator,
            &QCommunicator::worker_error);
    connect(m_communicator, &QCommunicator::operate_receiver, this,
            &QReceiver::init);
}

QReceiver::~QReceiver() {
    disconnect(this, &QReceiver::fetch_frame, m_communicator,
               &QCommunicator::fetch_frame_worker);
    disconnect(m_communicator, &QCommunicator::push_frame_worker, this,
               &QReceiver::push_frame);
    disconnect(this, &QReceiver::worker_done, m_communicator,
               &QCommunicator::worker_done);
    disconnect(this, &QReceiver::worker_error, m_communicator,
               &QCommunicator::worker_error);
    disconnect(m_communicator, &QCommunicator::operate_receiver, this,
               &QReceiver::init);
}

void QReceiver::init(std::shared_ptr<Can::Frame::Frame> frame) {
    init_timer();
    switch (frame->get_type()) {
        case Can::Frame::Type::SingleFrame: {
            DEBUG(info, "receiver single frame");
            m_frames = {frame};
            emit worker_done();
            break;
        }
        case Can::Frame::Type::FirstFrame: {
            DEBUG(info, "receiver first frame");
            m_frames.push_back(frame);
            m_consecutive_len =
                std::static_pointer_cast<Can::Frame::FirstFrame>(frame)->get_len();
            m_consecutive_last = 0x0;
            update_timer();
            emit fetch_frame(
                Can::Frame::FlowControl::build()
                    ->status(Can::Frame::FlowStatus::ContinueToSend)
                    ->block_size(0)
                    ->min_separation_time(0)
                    ->build()
                    .value());
            break;
        }
        default:
            DEBUG(info, "receiver error creating");
            emit worker_error(WorkerError::Other);
            break;
    }
}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
QReceiver::try_get_response() {
    DEBUG(info, "receiver");
    auto maybe_response = Can::frames_to_service(m_frames);
    if (!maybe_response) {
        DEBUG(error, "Invalid frames passed");
        return {};
    }
    return maybe_response.value();
}

void QReceiver::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    DEBUG(info, "QReceiver push_frame");
    switch (frame->get_type()) {
        case Can::Frame::Type::ConsecutiveFrame: {
            update_timer();
            DEBUG(info, "pushing consecutive frame");
            m_consecutive_last = (m_consecutive_last + 1) & 0xf;
            m_frames.push_back(frame);
            if (m_consecutive_last !=
                std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                    ->get_seq_num()) {
                emit worker_error(WorkerError::Other);
                break;
            }
            if (6 + (m_frames.size() - 1) * 7 >= m_consecutive_len) {
                emit worker_done();
            }
            break;
        }
        default:
            emit worker_error(WorkerError::Other);
            break;
    }
}
