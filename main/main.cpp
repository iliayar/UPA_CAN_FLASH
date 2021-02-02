#include "hex.h"

#include <iostream>
#include <QCanBus>
#include <QCanBusFrame>

int main(int argc, char *argv[])
{
    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        QStringLiteral("socketcan"), &errorString);
    if(!errorString.isEmpty())
        std::cerr << errorString.toStdString() << std::endl;
    else 
        for(auto device : devices) {
            std::cout << device.name().toStdString() << std::endl;
        }
}
