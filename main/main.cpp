#include <QApplication>
#include <iostream>
#include <memory>
#include <vector>

#include "mainwindow.h"
#include "frame.h"

Q_DECLARE_METATYPE(std::shared_ptr<Can::Frame>)
Q_DECLARE_METATYPE(std::vector<uint8_t>)

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame>>();
    
    MainWindow main_window{};
    main_window.show();

    return app.exec();
}
