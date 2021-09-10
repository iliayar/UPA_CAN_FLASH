#pragma once

#include <qcanbusdeviceinfo.h>
#include <qlist.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qtimer.h>

class DeviceCheker : public QObject {
    Q_OBJECT

public:
    DeviceCheker(QList<QCanBusDeviceInfo> devices, QList<int> bitrates);

signals:
    void connect_device(QString const& device_name, int bitrate);
    void device_status(bool active, QString const& device_name, int bitrate);
    void done();

public slots:
    void frame_recieved();
    void timeout();
    void next();

private:
    void next_impl();
    void send_status(bool active);
    int m_current_device;
    int m_current_bitrate;
    QList<QCanBusDeviceInfo> m_devices;
    QList<int> m_bitrates;
    QTimer m_timer;
};
