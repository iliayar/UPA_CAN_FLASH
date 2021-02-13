#include "flash.h"
#include "service_all.h"
#include "security.h"
#include "hex.h"
#include "crc.h"
#include "can.h"

#include <sstream>

using namespace Can;

#define IF_NEGATIVE(res) if(res->get_type() == Can::ServiceResponseType::Negative)

std::string int_to_hex(int n) {

    std::stringstream s;
    s << std::hex << n;    
    return s.str();
} 

void FlashTask::task() {
    ServiceResponse* response;

    response = call(new ServiceRequest_DiagnosticSessionControl(
        DiagnosticSessionControl_SubfunctionType::extendDiagnosticSession));

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot enter extendDiagnosticSession");
        return;
    }

    response = call(new ServiceRequest_ControlDTCSettings(
        ControlDTCSettings_SubfunctionType::off));

    IF_NEGATIVE(response) {
        LOG(error, "Failed ControlDTCSettings");
        return;
    }

    response = call(
        ServiceRequest_CommunicationControl::build()
            ->subfunction(CommunicationControl_SubfunctionType::disableRxAndTx)
            ->communication_type(CommunicationType::build()
                                     ->chanels(CommunicationTypeChanels::build()
                                                   ->network_communication(1)
                                                   ->normal_communication(1)
                                                   ->build())
                                     ->build())
            ->build());

    IF_NEGATIVE(response) {
        LOG(error, "Failed CommunicationControl");
        return;
    }

    response = call(
        ServiceRequest_DiagnosticSessionControl::build()
            ->subfunction(
                DiagnosticSessionControl_SubfunctionType::programmingSession)
            ->build());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot enter programmingSession");
        return;
    }

    uint8_t rnd = Crypto::get_RND();

    m_logger->info("Seed parameter " + int_to_hex(rnd));
    response =
        call(ServiceRequest_SecurityAccess::build()
                 ->subfunction(SecurityAccess_SubfunctionType::requestSeed)
                 ->seed_par(rnd)
                 ->build());


    IF_NEGATIVE(response) {
        LOG(error, "Failed ot request seed");
        return;
    }

    uint32_t seed =
        static_cast<ServiceResponse_SecurityAccess*>(response)->get_seed();

    LOG(info, "Received seed " + int_to_hex(seed));

    uint32_t key = Crypto::seed_to_key_02(seed, rnd);

    LOG(info, "Calculated key " + int_to_hex(key));

    response = call(ServiceRequest_SecurityAccess::build()
                        ->subfunction(SecurityAccess_SubfunctionType::sendKey)
                        ->key(key)
                        ->build());

    IF_NEGATIVE(response) {
        LOG(error, "Security access failed key verification");
        return;
    }

    LOG(info, "Successfully pased security access");

    std::ifstream fin(m_file);
    Hex::HexReader reader(new Hex::FileSource(fin));
    Hex::HexInfo hex_info = Hex::read_hex_info(reader);
    fin.close();
        
    uint32_t data_size = hex_info.size;
    uint32_t begin_address = hex_info.start_addr;
    uint16_t crc = hex_info.crc;
    
    std::vector<uint8_t> BEGIN_ADDRESS(4, 0);
    Util::Writer(BEGIN_ADDRESS).write_32(begin_address, 0, 32);
    std::vector<uint8_t> DATA_SIZE(4, 0);
    Util::Writer(DATA_SIZE).write_32(data_size, 0, 32);

    LOG(info, "Requesting download");

    response =
        call(ServiceRequest_RequestDownload::build()
                 ->data_format(DataFormatIdentifier::build()
                                   ->compressionMethod(0)
                                   ->encryptingMethod(0)
                                   ->build())
                 ->address_len_format(DataAndLengthFormatIdentifier::build()
                                          ->memory_address(4)
                                          ->memory_size(4)
                                          ->build())
                 ->memory_addr(BEGIN_ADDRESS)
                 ->memory_size(DATA_SIZE)
                 ->build());


    IF_NEGATIVE(response) {
       LOG(error, "Cannot request download");
       return; 
    }

    int block_length_fomat = static_cast<Can::ServiceResponse_RequestDownload*>(response)->get_length_format()->get_memory_size();
    if(block_length_fomat > 8) {
        LOG(error, "Too long max block length");
        return;
    }
    LOG(info, "Requesting download response parsed");
    std::vector<uint8_t> max_block_size_vec = static_cast<Can::ServiceResponse_RequestDownload*>(response)->get_max_blocks_number();
    LOG(info, "max_block_size_vec " + int_to_hex(max_block_size_vec.size()));
    uint64_t max_block_size = Util::Reader(max_block_size_vec).read_64(0, max_block_size_vec.size()*8);
    LOG(info, "max_block_size " + int_to_hex(max_block_size));
    max_block_size -= 2;
    fin.open(m_file);
    reader = Hex::HexReader(new Hex::FileSource(fin));
    LOG(info, "File " + m_file + " opened");
    std::vector<uint8_t> data(max_block_size, 0);
    int i = 0;
    int block_counter = 1;
    int n_size = 0;
    while(!reader.is_eof()) {
        Hex::HexLine *line = reader.read_line();
        if(line->get_type() == Hex::HexLineType::Data || line->get_type() == Hex::HexLineType::EndOfFile) {
                std::vector<uint8_t> line_data;
                if (line->get_type() == Hex::HexLineType::Data) {
                    line_data = static_cast<Hex::DataLine *>(line)->get_data();
                } else {
                    line_data = {data.back()};
                    data.resize(i);
                    i--;
                }
                for(uint8_t d : line_data) {
                    data[i++] = d;
                    n_size++;
                    if (i >= data.size()) {
                        LOG(important, "Transfering block " + int_to_hex(block_counter));
                        response =
                            call(Can::ServiceRequest_TransferData::build()
                                     ->block_counter(block_counter++)
                                     ->data(data)
                                     ->build());
                        IF_NEGATIVE(response) {
                            LOG(error, "Failed to transfer data");
                            return; 
                        }
                        if(static_cast<Can::ServiceResponse_TransferData*>(response)->get_block_counter() != block_counter - 1) {
                            LOG(warning, "Wrong block counter in response");
                        }
                        i = 0;
                    }
                }
        }
        if(line->get_type() == Hex::HexLineType::EndOfFile) {
            if(!reader.is_eof()) {
                LOG(warning, "EndOfFile in the middle of file"); 
            }
            break;
        }
    }
    fin.close();
    response = call(Can::ServiceRequest_RequestTransferExit::build()->build());
    IF_NEGATIVE(response) {
        LOG(error, "Failed to request transfer exit");
        return;
    }
    uint16_t crc_recv = static_cast<Can::ServiceResponse_RequestTransferExit*>(response)->get_crc();
    if(crc_recv != crc) {
        LOG(error, "CRC check failed");
        return;
    }
    LOG(info, "Flash done successfuly");
}
