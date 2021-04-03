#include "flash.h"

#include <sstream>

#include "crc.h"
#include "hex.h"
#include "qtask.h"
#include "security.h"
#include "service_all.h"

using namespace Can;

std::string int_to_hex(int n) {
    std::stringstream s;
    s << std::hex << n;
    return s.str();
}

void FlashTask::task() {
    task_main();
    std::shared_ptr<ServiceResponse::ServiceResponse> response =
        call(ServiceRequest::ECUReset::build()
                 ->subfunction(ServiceRequest::ECUReset::Subfunction::hardReset)
                 ->build()
                 .value());
    IF_NEGATIVE(response) { m_logger->error("Failed to hardReset device"); }
}
void FlashTask::task_main() {
    int progress = 0;
    std::shared_ptr<ServiceResponse::ServiceResponse> response;

    response = call(ServiceRequest::DiagnosticSessionControl::build()
                        ->subfunction(ServiceRequest::DiagnosticSessionControl::
                                          Subfunction::extendDiagnosticSession)
                        ->build()
                        .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot enter extendDiagnosticSession");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    response = call(
        ServiceRequest::ControlDTCSettings::build()
            ->subfunction(ServiceRequest::ControlDTCSettings::Subfunction::off)
            ->build()
            .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ControlDTCSettings");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    response = call(
        ServiceRequest::CommunicationControl::build()
            ->subfunction(ServiceRequest::CommunicationControl::Subfunction::
                              disableRxAndTx)
            ->communication_type(CommunicationType::build()
                                     ->chanels(CommunicationTypeChanels::build()
                                                   ->network_communication(1)
                                                   ->normal_communication(1)
                                                   ->build()
                                                   .value())
                                     ->build()
                                     .value())
            ->build()
            .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed CommunicationControl");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    response = call(ServiceRequest::DiagnosticSessionControl::build()
                        ->subfunction(ServiceRequest::DiagnosticSessionControl::
                                          Subfunction::programmingSession)
                        ->build()
                        .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed on enter programmingSession");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    uint8_t rnd = Crypto::get_RND();

    m_logger->info("Seed parameter " + int_to_hex(rnd));
    response =
        call(ServiceRequest::SecurityAccess::build()
                 ->subfunction(
                     ServiceRequest::SecurityAccess::Subfunction::requestSeed)
                 ->seed_par(rnd)
                 ->build()
                 .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot request seed");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    uint32_t seed =
        std::static_pointer_cast<ServiceResponse::SecurityAccess>(response)
            ->get_seed();

    LOG(info, "Received seed " + int_to_hex(seed));

    uint32_t key = Crypto::seed_to_key_02(seed, rnd);

    LOG(info, "Calculated key " + int_to_hex(key));

    response = call(
        ServiceRequest::SecurityAccess::build()
            ->subfunction(ServiceRequest::SecurityAccess::Subfunction::sendKey)
            ->key(key)
            ->build()
            .value());

    IF_NEGATIVE(response) {
        LOG(error, "Security access failed key verification");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    LOG(info, "Successfully passed security access");

    std::ifstream fin(m_file);
    if (!fin) {
        m_logger->error("Cannot open file");
        m_logger->progress(0, true);
        return;
    }
    Hex::HexReader reader(std::make_unique<Hex::FileSource>(fin));
    Hex::HexInfo hex_info = Hex::read_hex_info(reader).value();
    fin.close();

    uint32_t data_size = hex_info.size;
    uint32_t begin_address = hex_info.start_addr;
    uint16_t crc = hex_info.crc;

    Util::Writer writer(4);
    writer.write_int<uint32_t>(begin_address, 32);
    std::vector<uint8_t> BEGIN_ADDRESS = writer.get_payload();
    writer = Util::Writer(4);
    writer.write_int<uint32_t>(data_size, 32);
    std::vector<uint8_t> DATA_SIZE = writer.get_payload();

    LOG(info, "Requesting download");

    response =
        call(ServiceRequest::RequestDownload::build()
                 ->data_format(DataFormatIdentifier::build()
                                   ->compression_method(0)
                                   ->encryting_method(0)
                                   ->build()
                                   .value())
                 ->address_len_format(DataAndLengthFormatIdentifier::build()
                                          ->memory_address(4)
                                          ->memory_size(4)
                                          ->build()
                                          .value())
                 ->memory_addr(BEGIN_ADDRESS)
                 ->memory_size(DATA_SIZE)
                 ->build()
                 .value());

    IF_NEGATIVE(response) {
        LOG(error, "Cannot request download");
        m_logger->progress(0, true);
        return;
    }
    progress += 1;
    m_logger->progress(progress);

    int block_length_fomat =
        std::static_pointer_cast<Can::ServiceResponse::RequestDownload>(
            response)
            ->get_length_format()
            ->get_memory_size();
    if (block_length_fomat > 8) {
        LOG(error, "Too long max block length");
        m_logger->progress(0, true);
        return;
    }
    LOG(info, "Requesting download response parsed");
    std::vector<uint8_t> max_block_size_vec =
        std::static_pointer_cast<Can::ServiceResponse::RequestDownload>(
            response)
            ->get_max_blocks_number();
    LOG(info, "max_block_size_vec " + int_to_hex(max_block_size_vec.size()));
    uint64_t max_block_size =
        Util::Reader(max_block_size_vec)
            .read_int<uint64_t>(max_block_size_vec.size() * 8)
            .value();
    LOG(info, "max_block_size " + int_to_hex(max_block_size));
    max_block_size -= 2;
    fin.open(m_file);
    if (!fin) {
        m_logger->error("Cannot open file");
        m_logger->progress(0, true);
        return;
    }

    double progress_step = (100 - 7 - 1) / (hex_info.size / max_block_size);
    double transfer_progress = 0;
    reader = Hex::HexReader(std::make_shared<Hex::FileSource>(fin));
    LOG(info, "File " + m_file + " opened");
    std::vector<uint8_t> data(max_block_size, 0);
    int i = 0;
    int block_counter = 1;
    int n_size = 0;
    while (!reader.is_eof()) {
        std::shared_ptr<Hex::Line::Line> line = reader.read_line().value();
        if (line->get_type() == Hex::Line::Type::Data ||
            line->get_type() == Hex::Line::Type::EndOfFile) {
            std::vector<uint8_t> line_data;
            if (line->get_type() == Hex::Line::Type::Data) {
                line_data =
                    std::static_pointer_cast<Hex::Line::Data>(line)->get_data();
            } else {
                line_data = {data[i - 1]};
                data.resize(i);
                i--;
            }
            for (uint8_t d : line_data) {
                data[i++] = d;
                n_size++;
                if (i >= data.size()) {
                    LOG(important,
                        "Transfering block " + int_to_hex(block_counter));
                    response = call(Can::ServiceRequest::TransferData::build()
                                        ->block_counter(block_counter++)
                                        ->data(data)
                                        ->build()
                                        .value());
                    IF_NEGATIVE(response) {
                        LOG(error, "Failed to transfer data");
                        m_logger->progress(0, true);
                        return;
                    }
                    transfer_progress += progress_step;
                    m_logger->progress(progress + transfer_progress);
                    if (std::static_pointer_cast<
                            Can::ServiceResponse::TransferData>(response)
                            ->get_block_counter() != block_counter - 1) {
                        LOG(warning, "Wrong block counter in response");
                    }
                    i = 0;
                }
            }
        }
        if (line->get_type() == Hex::Line::Type::EndOfFile) {
            if (!reader.is_eof()) {
                LOG(warning, "EndOfFile in the middle of file");
            }
            break;
        }
    }
    fin.close();
    response = call(Can::ServiceRequest::RequestTransferExit::build()
                        ->crc(crc)
                        ->build()
                        .value());
    IF_NEGATIVE(response) {
        LOG(error, "Failed to request transfer exit. Maybe crc check failed");
        m_logger->progress(0, true);
        return;
    }
    m_logger->progress(100);
    LOG(info, "Flash done successfuly");
}
