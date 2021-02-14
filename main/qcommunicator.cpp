#include "qcommunicator.h"

#include <QObject>
#include <QThread>
#include <memory>
#include <stdexcept>

#include "frame_all.h"
#include "service.h"
#include "task.h"

void QCommunicator::set_task(QTask* task) {
    DEBUG(info, "QCommunicator setting task");
    if (m_task != nullptr) {
        m_task->terminate();
        disconnect(this, &QCommunicator::response, task, &QTask::response);
        disconnect(task, &QTask::request, this, &QCommunicator::request);
        delete m_task;
    }
    m_task = task;
    connect(this, &QCommunicator::response, task, &QTask::response);
    connect(task, &QTask::request, this, &QCommunicator::request);
    m_task->start();
}

void QCommunicator::push_frame(std::shared_ptr<Can::Frame> frame) {
    m_logger->received_frame(frame);
    DEBUG(info, "QCommunicator push_frame");
    if (m_worker != nullptr) {
        emit push_frame_worker(frame);
        // :TODO: Check if worker done
        return;
    }
    QReceiver* worker = new QReceiver;
    connect(worker, &QReceiver::fetch_frame, this,
            &QCommunicator::fetch_frame_worker);
    connect(this, &QCommunicator::push_frame_worker, worker,
            &QReceiver::push_frame);
    connect(worker, &QReceiver::worker_done, this, &QCommunicator::worker_done);
    connect(worker, &QReceiver::worker_error, this,
            &QCommunicator::worker_error);
    connect(this, &QCommunicator::operate_receiver, worker, &QReceiver::init);
    m_worker = worker;
    m_worker->moveToThread(&m_worker_thread);
    // m_worker_thread.start();
    emit operate_receiver(frame);
    // m_worker_thread.start();
    DEBUG(info, "QReceiver created");
}

void QCommunicator::fetch_frame_worker(std::shared_ptr<Can::Frame> frame) {
    DEBUG(info, "QCommunicator fetch_frame_worker");
    m_logger->transmitted_frame(frame);
    emit fetch_frame(frame);
}

void QCommunicator::request(Can::ServiceRequest* r) {
    if (m_worker != nullptr) {
        // :FIXME: throw error? worer busy
    }
    DEBUG(info, "QCommunicator receive request");
    QTransmitter* worker = new QTransmitter;
    connect(this, &QCommunicator::push_frame_worker, worker,
            &QTransmitter::push_frame);
    connect(worker, &QTransmitter::fetch_frame, this,
            &QCommunicator::fetch_frame_worker);
    connect(worker, &QTransmitter::worker_done, this,
            &QCommunicator::worker_done);
    connect(worker, &QTransmitter::worker_error, this,
            &QCommunicator::worker_error);
    connect(this, &QCommunicator::operate_transmitter, worker,
            &QTransmitter::init);
    m_worker = worker;

    m_worker->moveToThread(&m_worker_thread);
    // m_worker_thread.start();
    emit operate_transmitter(r);
    // worker->init(r);
    DEBUG(info, "QTransmitter created");
}

void QCommunicator::worker_error() { m_logger->error("Worker error"); }
void QCommunicator::worker_done() {
    DEBUG(info, "QCommunicator worker_done");
    switch (m_worker->get_type()) {
        case Can::CommunicatorStatus::Receive: {
            QReceiver* worker = static_cast<QReceiver*>(m_worker);
            emit response(worker->get_response());
            // m_worker_thread.quit();
            // m_worker_thread.wait();
            disconnect(worker, &QReceiver::fetch_frame, this,
                       &QCommunicator::fetch_frame_worker);
            disconnect(this, &QCommunicator::push_frame_worker, worker,
                       &QReceiver::push_frame);
            disconnect(worker, &QReceiver::worker_done, this,
                       &QCommunicator::worker_done);
            disconnect(worker, &QReceiver::worker_error, this,
                       &QCommunicator::worker_error);
            disconnect(this, &QCommunicator::operate_receiver, worker,
                       &QReceiver::init);
            delete static_cast<QReceiver*>(m_worker);
            m_worker = nullptr;
        }
        case Can::CommunicatorStatus::Transmit: {
            QTransmitter* worker = static_cast<QTransmitter*>(m_worker);
            // m_worker_thread.quit();
            // m_worker_thread.wait();
            disconnect(worker, &QTransmitter::fetch_frame, this,
                       &QCommunicator::fetch_frame_worker);
            disconnect(this, &QCommunicator::push_frame_worker, worker,
                       &QTransmitter::push_frame);
            disconnect(worker, &QTransmitter::worker_done, this,
                       &QCommunicator::worker_done);
            disconnect(worker, &QTransmitter::worker_error, this,
                       &QCommunicator::worker_error);
            disconnect(this, &QCommunicator::operate_transmitter, worker,
                       &QTransmitter::init);
            delete static_cast<QTransmitter*>(m_worker);
            m_worker = nullptr;
        }
        default: {
            if (m_worker != nullptr) {
                // This not happen
            }
        }
    }
    DEBUG(info, "QCommunicator worker_done quit");
}

void QTransmitter::init(Can::ServiceRequest* request) {
    m_frames = Can::service_to_frames(request);
    if (m_frames.size() == 0) {
        throw std::runtime_error("Failed to disassemble request to frames");
    }
    switch (m_frames[0]->get_type()) {
        case Can::FrameType::SingleFrame: {
            DEBUG(info, "QTransmitter fetch single frame");
            emit fetch_frame(m_frames[0]);
            emit worker_done();
            break;
        }
        case Can::FrameType::FirstFrame: {
            DEBUG(info, "transmitter first frame");
            m_i = 1;
            emit fetch_frame(m_frames[0]);
            break;
        }
        default:
            DEBUG(info, "transmitter error frame");
            emit worker_error();
            break;
    }
}

void QTransmitter::push_frame(std::shared_ptr<Can::Frame> frame) {
    DEBUG(info, "QTransmitter push_frame");
    if (frame->get_type() == Can::FrameType::FlowControl) {
        DEBUG(info, "transmitter pushing with m_wait_fc and flow control frame")
        Can::FlowStatus status =
            static_cast<Can::Frame_FlowControl*>(frame.get())->get_status();
        if (status ==
            Can::FlowStatus::WaitForAnotherFlowControlMessageBeforeContinuing) {
            return;
        } else if (status == Can::FlowStatus::OverflowAbortTransmission) {
            emit worker_error();
            return;
        }
        int fc_block_size =
            static_cast<Can::Frame_FlowControl*>(frame.get())->get_block_size();
        if (fc_block_size == 0) fc_block_size = m_frames.size();
        int fc_min_time = static_cast<Can::Frame_FlowControl*>(frame.get())
                              ->get_min_separation_time();
        for (int i = 0; i < fc_block_size && m_i < m_frames.size();
             ++i, ++m_i) {
            QThread::msleep(fc_min_time);
            emit fetch_frame(m_frames[m_i]);
        }
        if (m_i >= m_frames.size()) emit worker_done();
    }
}

void QReceiver::init(std::shared_ptr<Can::Frame> frame) {
    switch (frame->get_type()) {
        case Can::FrameType::SingleFrame: {
            DEBUG(info, "receiver single frame");
            m_frames = {frame};
            emit worker_done();
            break;
        }
        case Can::FrameType::FirstFrame: {
            DEBUG(info, "receiver first frame");
            m_frames.push_back(frame);
            m_consecutive_len =
                static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len();
            m_consecutive_last = 0x0;
            emit fetch_frame(std::make_shared<Can::Frame_FlowControl>(
                Can::FlowStatus::ContinueToSend, 0, 0));
            break;
        }
        default:
            DEBUG(info, "receiver error creating");
            emit worker_error();
            break;
    }
}

Can::ServiceResponse* QReceiver::get_response() {
    DEBUG(info, "receiver");
    Can::ServiceResponse* response = Can::frames_to_service(m_frames);
    return response;
}

void QReceiver::push_frame(std::shared_ptr<Can::Frame> frame) {
    DEBUG(info, "QReceiver push_frame");
    switch (frame->get_type()) {
        case Can::FrameType::ConsecutiveFrame: {
            DEBUG(info, "pushing consecutive frame");
            m_consecutive_last = (m_consecutive_last + 1) & 0xf;
            m_frames.push_back(frame);
            if (m_consecutive_last !=
                static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())
                    ->get_seq_num()) {
                emit worker_error();
                break;
            }
            update_imp();
            if (6 + (m_frames.size() - 1) * 7 >= m_consecutive_len) {
                emit worker_done();
            }
            break;
        }
        default:
            emit worker_error();
            break;
    }
}
