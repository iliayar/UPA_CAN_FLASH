
#include "gtest/gtest.h"
#include "service_all.h"

#include <vector>

TEST(testService, testData)
{
    {
        Can::Data* data = Can::DataFactory(std::vector<uint8_t>({0x20, 0x0e, 0x03})).get();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::UPASystemType);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x03}));
    }
}

TEST(testService, testServiceRequest)
{
    {
        Can::ServiceRequest_ReadDataByIdentifier request(Can::DataIdentifier::VIN);
        std::vector<uint8_t> res = {0x22, 0xf1, 0x90};
        EXPECT_EQ(res, request.dump());
    }

    {
        Can::ServiceRequest_WriteDataByIdentifier request(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>(17, 0x41)));
        std::vector<uint8_t> res = {0x2e, 0xf1, 0x90};
        res.resize(3 + 17, 0x41);
        EXPECT_EQ(res, request.dump());
    }
}

TEST(testService, testServiceResponse)
{
    {
        Can::ServiceResponse* response = Can::ServiceResponseFactory(std::vector<uint8_t>({0x62, 0x20, 0x0E, 0x03, 0x55, 0x55, 0x55})).get();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::ReadDataByIdentifier);
        Can::Data* data = static_cast<Can::ServiceResponse_ReadDataByIdentifier*>(response)->get_data();
        EXPECT_NE(data, nullptr);
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::UPASystemType);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x03}));
    }

    {
        Can::ServiceResponse* response = Can::ServiceResponseFactory(std::vector<uint8_t>({0x6e, 0xf1, 0x90, 0x55, 0x55, 0x55, 0x55})).get();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::WriteDataByIdentifier);
        Can::DataIdentifier id = static_cast<Can::ServiceResponse_WriteDataByIdentifier*>(response)->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }
}
