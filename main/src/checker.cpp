#include "checker.h"

#include <qcanbusdeviceinfo.h>
#include <qlist.h>
#include <qtimer.h>
#include <string>
#include "logger.h"

DeviceCheker::DeviceCheker(QList<QCanBusDeviceInfo> devices,
                           QList<int> bitrates)
    : m_bitrates(bitrates),
      m_devices(devices),
      m_current_device(0),
      m_current_bitrate(0) {
    connect(&m_timer, &QTimer::timeout, this, &DeviceCheker::timeout);
    m_timer.setSingleShot(true);
}

void DeviceCheker::frame_recieved() {
    send_status(true);
}

void DeviceCheker::timeout() {
    send_status(false);
}

void DeviceCheker::next_impl() {
    m_current_bitrate = (m_current_bitrate + 1) % m_bitrates.size();
    if(m_current_bitrate == 0) {
        m_current_device++;
    }
}

void DeviceCheker::send_status(bool active) {
    m_timer.stop();
    QString device_name = m_devices[m_current_device].name();
    int bitrate = m_bitrates[m_current_bitrate];
    next_impl();
    emit device_status(active, device_name, bitrate);
}

void DeviceCheker::next() {
    if(m_current_device >= m_devices.size()) {
        emit done();
        return;
    }

    DEBUG(info, "device: " + std::to_string(m_current_device) +
                    ", bitrate: " + std::to_string(m_current_bitrate));

    m_timer.start(250);
    emit connect_device(m_devices[m_current_device].name(), m_bitrates[m_current_bitrate]);
}
