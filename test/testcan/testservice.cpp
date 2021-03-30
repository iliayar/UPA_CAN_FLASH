
#include <vector>

#include "gtest/gtest.h"
#include "service_all.h"

TEST(testService, testDataUPASystemType) {
    Util::Reader reader({0x20, 0x0e, 0x03});
    auto maybe_data = Can::Data::build(reader)->build();
    EXPECT_TRUE(maybe_data);
    auto data = maybe_data.value();
    EXPECT_EQ(data->get_type(), Can::DataIdentifier::UPASystemType);
    EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x03}));
}

TEST(testService, testServiceRequestReadDataByIdentifier) {
    auto maybe_request = Can::ServiceRequest::ReadDataByIdentifier::build()
                             ->id(Can::DataIdentifier::VIN)
                             ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x22, 0xf1, 0x90};
    EXPECT_EQ(res, request->dump().value());
}

TEST(testService, testServiceRequestWiteDataByIndentifier) {
    auto maybe_data = Can::Data::build()
                          ->type(Can::DataIdentifier::VIN)
                          ->value(std::vector<uint8_t>(17, 0x41))
                          ->build();
    EXPECT_TRUE(maybe_data);
    auto data = maybe_data.value();
    auto maybe_request = Can::ServiceRequest::WriteDataByIdentifier::build()
                             ->data(data)
                             ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x2e, 0xf1, 0x90};
    res.resize(3 + 17, 0x41);
    EXPECT_EQ(res, request->dump().value());
}
TEST(testService, testServiceRequestSecutiryAccessRequestSeed) {
    auto maybe_request =
        Can::ServiceRequest::SecurityAccess::build()
            ->subfunction(
                Can::ServiceRequest::SecurityAccess::Subfunction::requestSeed)
            ->key(0)
            ->seed_par(0x42)
            ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x27, 0x03, 0x42};
    EXPECT_EQ(res, request->dump().value());
}
TEST(testService, testServiceRequestSecutiryAccessSendKey) {
    auto maybe_request =
        Can::ServiceRequest::SecurityAccess::build()
            ->subfunction(
                Can::ServiceRequest::SecurityAccess::Subfunction::sendKey)
            ->seed_par(0)
            ->key(0x13370132)
            ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x27, 0x04, 0x13, 0x37, 0x01, 0x32};
    EXPECT_EQ(res, request->dump().value());
}
TEST(testService, testServiceRequestRequestDownload) {
    auto format = Can::DataFormatIdentifier::build()
                      ->compression_method(0)
                      ->encryting_method(0)
                      ->build();
    EXPECT_TRUE(format);
    auto data_len = Can::DataAndLengthFormatIdentifier::build()
                        ->memory_address(0x04)
                        ->memory_size(0x04)
                        ->build();
    EXPECT_TRUE(data_len);
    auto maybe_request = Can::ServiceRequest::RequestDownload::build()
                             ->data_format(format.value())
                             ->address_len_format(data_len.value())
                             ->memory_addr({0x08, 0x00, 0x40, 0x00})
                             ->memory_size({0x00, 0x00, 0x90, 0xE8})
                             ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x34, 0x00, 0x44, 0x08, 0x00, 0x40,
                                0x00, 0x00, 0x00, 0x90, 0xE8};
    EXPECT_EQ(res, request->dump().value());
}
TEST(testService, testServiceRequestTransferData) {
    auto maybe_request = Can::ServiceRequest::TransferData::build()
                             ->block_counter(0x32)
                             ->data({0x27, 0x04, 0x13, 0x37, 0x01, 0x32})
                             ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x36, 0x32, 0x27, 0x04,
                                0x13, 0x37, 0x01, 0x32};
    EXPECT_EQ(res, request->dump().value());
}
TEST(testService, testServiceRequestCommunicationControl) {
    auto chanels = Can::CommunicationTypeChanels::build()
                       ->network_communication(1)
                       ->normal_communication(1)
                       ->build();
    EXPECT_TRUE(chanels);
    auto communication =
        Can::CommunicationType::build()->chanels(chanels.value())->build();
    EXPECT_TRUE(communication);
    auto maybe_request =
        Can::ServiceRequest::CommunicationControl::build()
            ->subfunction(Can::ServiceRequest::CommunicationControl::
                              Subfunction::disableRxAndTx)
            ->communication_type(communication.value())
            ->build();
    EXPECT_TRUE(maybe_request);
    auto request = maybe_request.value();
    std::vector<uint8_t> res = {0x28, 0x03, 0x03};
    EXPECT_EQ(res, request->dump().value());
}

TEST(testService, testServiceResponseNegative) {
    auto maybe_response =
        Can::ServiceResponse::Factory({0x7f, 0x27, 0x10}).get();
    EXPECT_TRUE(maybe_response);
    auto response = maybe_response.value();
    EXPECT_EQ(response->get_type(), Can::ServiceResponse::Type::Negative);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::Negative>(response)
                  ->get_service(),
              Can::ServiceRequest::Type::SecurityAccess);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::Negative>(response)
                  ->get_code(),
              0x10);
}

TEST(testService, testServiceResponseReadDataByIdentifier) {
    auto maybe_response = Can::ServiceResponse::Factory({0x62, 0x20, 0x0E,
    0x03, 0x55, 0x55, 0x55}).get(); EXPECT_TRUE(maybe_response); auto
    response = maybe_response.value(); EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::ReadDataByIdentifier);
    auto data =
        std::static_pointer_cast<Can::ServiceResponse::ReadDataByIdentifier>(response)
            ->get_data();
    EXPECT_NE(data, nullptr);
    EXPECT_EQ(data->get_type(), Can::DataIdentifier::UPASystemType);
    EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x03}));
}

TEST(testService, testServiceResponseWriteDataByIdentifier) {
    auto maybe_response = Can::ServiceResponse::Factory({0x6e, 0xf1, 0x90,
    0x55, 0x55, 0x55, 0x55}).get(); EXPECT_TRUE(maybe_response); auto
    response = maybe_response.value(); EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::WriteDataByIdentifier);
    Can::DataIdentifier id =
        std::static_pointer_cast<Can::ServiceResponse::WriteDataByIdentifier>(response)
            ->get_id();
    EXPECT_EQ(id, Can::DataIdentifier::VIN);
}

TEST(testService, testServiceResponseSecurityAccessRequestSeed) {
    auto maybe_response = Can::ServiceResponse::Factory({0x67, 0x03, 0x13,
    0x37, 0x01, 0x32}).get(); EXPECT_TRUE(maybe_response); auto response =
    maybe_response.value(); EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::SecurityAccess);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::SecurityAccess>(response)
                  ->get_subfunction(),
              Can::ServiceResponse::SecurityAccess::Subfunction::requestSeed);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::SecurityAccess>(response)
                  ->get_seed(),
              0x13370132);
}

TEST(testService, testServiceResponseSecurityAccessSendKey) {
    auto maybe_response = Can::ServiceResponse::Factory({0x67, 0x04}).get();
    EXPECT_TRUE(maybe_response);
    auto response = maybe_response.value();
    EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::SecurityAccess);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::SecurityAccess>(response)
                  ->get_subfunction(),
              Can::ServiceResponse::SecurityAccess::Subfunction::sendKey);
}

TEST(testService, testServiceResponseTransferData) {
    auto maybe_response = Can::ServiceResponse::Factory({0x76, 0x32, 0x67,
    0x03, 0x13, 0x37, 0x01, 0x32}).get(); EXPECT_TRUE(maybe_response); auto
    response = maybe_response.value(); EXPECT_EQ(response->get_type(),
    Can::ServiceResponse::Type::TransferData);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::TransferData>(response)
                  ->get_block_counter(),
              0x32);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::TransferData>(response)
                  ->get_data(),
              std::vector<uint8_t>({0x67, 0x03, 0x13, 0x37, 0x01, 0x32}));
}

TEST(testService, testServiceResponseRequestDownload) {
    auto maybe_response = Can::ServiceResponse::Factory({0x74, 0x20, 0x04,
    0x02}).get(); EXPECT_TRUE(maybe_response); auto response =
    maybe_response.value(); EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::RequestDownload);
    std::shared_ptr<Can::LengthFormatIdentifier> format =
        std::static_pointer_cast<Can::ServiceResponse::RequestDownload>(response)
            ->get_length_format();
    EXPECT_EQ(format->get_memory_size(), 0x02);
    EXPECT_EQ(std::static_pointer_cast<Can::ServiceResponse::RequestDownload>(response)
                  ->get_max_blocks_number(),
              std::vector<uint8_t>({0x04, 0x02}));
}

TEST(testService, testServiceResponseRequestTransferExit) {
    auto maybe_response = Can::ServiceResponse::Factory({0x77, 0x01,
    0x02}).get(); EXPECT_TRUE(maybe_response); auto response =
    maybe_response.value(); EXPECT_EQ(response->get_type(),
              Can::ServiceResponse::Type::RequestTransferExit);
    EXPECT_EQ(
        std::static_pointer_cast<Can::ServiceResponse::RequestTransferExit>(response)
            ->get_crc(),
        0x0102);
}
