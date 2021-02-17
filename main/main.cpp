#include <QApplication>
#include <QMessageBox>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "mainwindow.h"
#include "frame.h"
#include "service.h"
#include "qtask.h"

#include <QTextCharFormat>
#include <QTextCursor>
Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

Q_DECLARE_METATYPE(std::shared_ptr<Can::Frame>)
Q_DECLARE_METATYPE(std::shared_ptr<QTask>)
Q_DECLARE_METATYPE(std::shared_ptr<Can::ServiceRequest>)
Q_DECLARE_METATYPE(std::shared_ptr<Can::ServiceResponse>)
Q_DECLARE_METATYPE(std::vector<uint8_t>)
Q_DECLARE_METATYPE(Can::ServiceRequest*)
Q_DECLARE_METATYPE(Can::ServiceResponse*)
Q_DECLARE_METATYPE(WorkerError)
Q_DECLARE_METATYPE(std::string)

#define STR(a) #a

class Application : public QApplication  {
public:
    Application(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject *receiver, QEvent* e) override {
        bool done = true;
        try {
            done = QApplication::notify(receiver, e);
        } catch(std::runtime_error e) {
            m_box.critical(nullptr, "Error", e.what());
        }
        return true;
    }

private:

    QMessageBox m_box;
};


int main(int argc, char *argv[]) {
    Application app(argc, argv);
    app.setApplicationName("UPA_CAN_FLASH " APP_VERSION);

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame>>();
    qRegisterMetaType<std::shared_ptr<QTask>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceRequest>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceResponse>>();
    qRegisterMetaType<Can::ServiceRequest*>();
    qRegisterMetaType<Can::ServiceRequest*>();
    qRegisterMetaType<std::string>();
    qRegisterMetaType<WorkerError>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();
    
    MainWindow main_window{};
    main_window.show();

    return app.exec();
}
