#include <QApplication>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "mainwindow.h"
#include "frame.h"
#include "service.h"

#include <QTextCharFormat>
#include <QTextCursor>
Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

Q_DECLARE_METATYPE(std::shared_ptr<Can::Frame>)
Q_DECLARE_METATYPE(std::vector<uint8_t>)
Q_DECLARE_METATYPE(Can::ServiceRequest*)
Q_DECLARE_METATYPE(Can::ServiceResponse*)
Q_DECLARE_METATYPE(std::string)

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame>>();
    qRegisterMetaType<Can::ServiceRequest*>();
    qRegisterMetaType<Can::ServiceResponse*>();
    qRegisterMetaType<std::string>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();
    
    MainWindow main_window{};
    main_window.show();

    return app.exec();
}
