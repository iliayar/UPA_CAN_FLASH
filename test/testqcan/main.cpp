#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "gtest/gtest.h"
#include "qtask.h"

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
Q_DECLARE_METATYPE(std::string)

struct smersh {
    bool KillAppAfterTimeout(int secs = 10) const;
};

bool smersh::KillAppAfterTimeout(int secs) const {
    QScopedPointer<QTimer> timer(new QTimer);
    timer->setSingleShot(true);
    bool ok = timer->connect(timer.data(), SIGNAL(timeout()), qApp,
                             SLOT(quit()), Qt::QueuedConnection);
    timer->start(secs * 1000);  // N seconds timeout
    timer.take()->setParent(qApp);
    return ok;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType<std::vector<uint8_t>>();
    qRegisterMetaType<std::shared_ptr<Can::Frame::Frame>>();
    qRegisterMetaType<std::shared_ptr<QTask>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceRequest::ServiceRequest>>();
    qRegisterMetaType<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>();
    qRegisterMetaType<Can::ServiceRequest::ServiceRequest*>();
    qRegisterMetaType<Can::ServiceRequest::ServiceRequest*>();
    qRegisterMetaType<std::string>();
    qRegisterMetaType<QTextCharFormat>();
    qRegisterMetaType<QTextCursor>();
    qRegisterMetaType<std::vector<std::shared_ptr<Can::DTC>>>();

    int iReturn = 1;
    QTimer timer;
    QTimer killTimer;
    killTimer.setSingleShot(true);
    killTimer.connect(&killTimer, &QTimer::timeout, [&]() {
        a.quit();
    });
    timer.setSingleShot(true);
    timer.connect(&timer, &QTimer::timeout, [&]() {
        ::testing::InitGoogleTest(&argc, argv);
        int iReturn = RUN_ALL_TESTS();
    });
    killTimer.start(2000);
    timer.start(1000);
    a.exec();
    return iReturn;
}
