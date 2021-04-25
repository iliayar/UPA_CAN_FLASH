#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTextCharFormat>
#include <QTextCursor>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "frame.h"
#include "mainwindow.h"
#include "qtask.h"
#include "service.h"

Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

Q_DECLARE_METATYPE(std::shared_ptr<Can::Frame::Frame>)
Q_DECLARE_METATYPE(std::shared_ptr<QTask>)
Q_DECLARE_METATYPE(std::shared_ptr<Can::ServiceRequest::ServiceRequest>)
Q_DECLARE_METATYPE(std::shared_ptr<Can::ServiceResponse::ServiceResponse>)
Q_DECLARE_METATYPE(std::vector<uint8_t>)
Q_DECLARE_METATYPE(std::vector<std::shared_ptr<Can::DTC>>)
Q_DECLARE_METATYPE(Can::ServiceRequest::ServiceRequest*)
Q_DECLARE_METATYPE(Can::ServiceResponse::ServiceResponse*)
Q_DECLARE_METATYPE(WorkerError)
Q_DECLARE_METATYPE(std::string)

#define STR(a) #a

class Application : public QApplication {
public:
    Application(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* e) override {
        try {
            QApplication::notify(receiver, e);
            return true;
        } catch (std::experimental::bad_optional_access e) {
            m_box.critical(nullptr, "Error", e.what());
            std::cout << e.what() << std::endl;
            return false;
        }
    }

private:
    QMessageBox m_box;
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    app.setApplicationName("UPA_CAN_FLASH " APP_VERSION);
    app.setStyle(QStyleFactory::create("Fusion"));

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame::Frame>>();
    qRegisterMetaType<std::shared_ptr<QTask>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceRequest::ServiceRequest>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>();
    qRegisterMetaType<Can::ServiceRequest::ServiceRequest*>();
    qRegisterMetaType<Can::ServiceRequest::ServiceRequest*>();
    qRegisterMetaType<std::string>();
    qRegisterMetaType<WorkerError>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();
    qRegisterMetaType<std::vector<std::shared_ptr<Can::DTC>>>();

    QDesktopWidget dw;
    MainWindow main_window{};

    main_window.resize(dw.availableGeometry(&main_window).size() * 0.8);

    main_window.show();

    return app.exec();
}
