#include <QApplication>
#include <iostream>
#include <memory>
#include <vector>

#include "mainwindow.h"
#include "frame.h"

#include <QTextCharFormat>
#include <QTextCursor>
Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCursor)

Q_DECLARE_METATYPE(std::shared_ptr<Can::Frame>)
Q_DECLARE_METATYPE(std::vector<uint8_t>)

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame>>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();
    
    MainWindow main_window{};
    main_window.show();

    return app.exec();
}
