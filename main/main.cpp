#include "hex.h"

#include <iostream>
#include <QCanBus>
#include <QCanBusFrame>

#ifdef __MINGW32__
#define CAN_PLUGIN "systeccan"
#elif __linux__
#define CAN_PLUGIN "socketcan"
#endif

int main(int argc, char *argv[])
{
    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        QStringLiteral(CAN_PLUGIN), &errorString);
    if(!errorString.isEmpty())
        std::cerr << errorString.toStdString() << std::endl;
    else 
        for(auto device : devices) {
            std::cout << device.name().toStdString() << std::endl;
        }
}
