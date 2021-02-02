#pragma once

#include <QMainWindow>
#include <QCanBus>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void processReceivedFrames();
private:
    QCanBusDevice *m_device;
};
