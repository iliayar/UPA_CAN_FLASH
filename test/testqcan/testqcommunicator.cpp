#include "testqcommunicator.h"

#include "gtest/gtest.h"
#include "logger.h"
#include "qcommunicator.h"
#include "qlogger.h"
#include "qtask.h"
#include "frame_all.h"


TEST(testQCommunicator, testNoDelays) {
    std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
    frames.push_back(Can::Frame::FirstFrame::build()
                         ->len(20)
                         ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(1)
                         ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(2)
                         ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                         ->build()
                         .value());
    TestQLoggerWorker logger;
    std::shared_ptr<QLogger> dummy_logger = std::make_shared<QLogger>(&logger);
    TestWindow window(dummy_logger);
    std::shared_ptr<QTask> task = std::make_shared<TestTask>(dummy_logger);
    window.test(task, frames, {100, 200, 300});

    EXPECT_EQ(logger.m_received_frames.size(), 3);
    EXPECT_EQ(logger.m_transmitted_frames.size(), 1);
    EXPECT_EQ(logger.m_infos, 2);
    EXPECT_EQ(logger.m_errors, 2);
    EXPECT_EQ(logger.m_warnings, 2);
}
