/*
 * SoftwareUpdateService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: stefanosperett
 */

#include "SoftwareUpdateService.h"

extern DSerial serial;

/**
 *
 *   Process the Service (Called by CommandHandler)
 *
 *   Parameters:
 *   PQ9Frame &command          Frame received over the bus
 *   DataBus &interface       Bus object
 *   PQ9Frame &workingBuffer    Reference to buffer to store the response.
 *
 *   Returns:
 *   bool true      :           Frame is directed to this Service
 *        false     :           Frame is not directed to this Service
 *
 */

SoftwareUpdateService::SoftwareUpdateService(MB85RS &fram_in) {
    fram = &fram_in;
}

bool SoftwareUpdateService::process(DataFrame &command, DataBus &interface, DataFrame &workingBuffer) {
    if (command.getPayload()[COMMAND_SERVICE] == SOFTWAREUPDATE_SERVICE) {
        // prepare response frame
        workingBuffer.setDestination(command.getSource());
        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setPayloadSize(PAYLOAD_SIZE_OFFSET);
        workingBuffer.getPayload()[COMMAND_SERVICE] = SOFTWAREUPDATE_SERVICE;
        workingBuffer.getPayload()[COMMAND_RESPONSE] = COMMAND_RESPONSE;
        workingBuffer.getPayload()[COMMAND_METHOD] = command.getPayload()[COMMAND_METHOD];

        payload_data = workingBuffer.getPayload();
        payload_size = PAYLOAD_SIZE_OFFSET;

        if(command.getPayload()[COMMAND_METHOD] != ERASE_SLOT) state_flags &= ~ERASE_FLAG;

        switch (command.getPayload()[COMMAND_METHOD]) {
        case START_OTA:
            if(command.getPayloadSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                    start_OTA(command.getPayload()[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nOTA started!");

                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_METADATA:
            if(command.getPayloadSize() == METADATA_SIZE - 1 + PAYLOAD_SIZE_OFFSET) {
                receive_metadata(&(command.getPayload()[COMMAND_DATA]));
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nMetadata received!");

            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SEND_METADATA:
            if(command.getPayloadSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                    send_metadata(command.getPayload()[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) {
                        print_metadata(&payload_data[COMMAND_DATA]);
                        serial.println("\nMetadata sended!");
                    }
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_PARTIAL_CRCS:
            if(command.getPayloadSize() <= BLOCK_SIZE + PAYLOAD_SIZE_OFFSET) {
                receive_partial_crcs(&(command.getPayload()[COMMAND_DATA]), command.getPayloadSize() - PAYLOAD_SIZE_OFFSET);
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nPartial crc block received!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_BLOCK:
            if(command.getPayloadSize() <= BLOCK_SIZE + 2 + PAYLOAD_SIZE_OFFSET) {
                receive_block(&(command.getPayload()[COMMAND_DATA + 2]), command.getPayload()[COMMAND_DATA] | (command.getPayload()[COMMAND_DATA + 1] << 8));
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nBlock received!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case CHECK_MD5:
            if(command.getPayloadSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                    check_md5(command.getPayload()[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nMD5 is correct!");
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case STOP_OTA:
            if(command.getPayloadSize() == PAYLOAD_SIZE_OFFSET) {
                stop_OTA();
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nOTA is stopped!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case ERASE_SLOT:
            if(command.getPayloadSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if((state_flags & ERASE_FLAG) == 0) {
                    if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                        slot_erase = command.getPayload()[COMMAND_DATA];
                        state_flags |= ERASE_FLAG;
                        serial.println("Are you sure(13)?");
                    } else throw_error(SLOT_OUT_OF_RANGE);
                } else {
                    if(command.getPayload()[COMMAND_DATA] == ACKNOWLEDGE) {
                        erase_slot(slot_erase - 1);
                        if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nSlot is erased!");
                    } else throw_error(PARAMETER_MISMATCH);
                }
            } else throw_error(PARAMETER_MISMATCH);
            break;

        default:
            break;
        }

        workingBuffer.setPayloadSize(payload_size);
        print_response();
        // command processed
        return true;
    } else {
        // this command is related to another service,
        // report the command was not processed
        return false;
    }
}

void SoftwareUpdateService::start_OTA(unsigned char slot_number) {
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) == 0) {
        unsigned char state;
        if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
        fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &state, 1);

        if(state == EMPTY) {
            state_flags |= UPDATE_FLAG;
            state_flags &= ~METADATA_FLAG;
            update_slot = slot_number;
            state = PARTIAL;
            if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
            fram->write((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &state, 1);
        } else if(state == PARTIAL) {
            state_flags |= UPDATE_FLAG | METADATA_FLAG;
            update_slot = slot_number;
        } else return throw_error(SLOT_NOT_EMPTY);
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void SoftwareUpdateService::receive_metadata(unsigned char* metadata) {
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) == 0) {
            if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
            fram->write((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + 1, metadata, METADATA_SIZE - 1);
            num_update_blocks = metadata[NUM_BLOCKS_OFFSET - 1] | (metadata[NUM_BLOCKS_OFFSET] << 8);
            state_flags |= METADATA_FLAG;
        } else return throw_error(METADATA_ALREADY_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::send_metadata(unsigned char slot_number) {
    payload_size = PAYLOAD_SIZE_OFFSET + METADATA_SIZE ;

    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &payload_data[COMMAND_DATA], METADATA_SIZE);
}

void SoftwareUpdateService::receive_partial_crcs(unsigned char* crc_block, unsigned char num_bytes) {
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if(received_par_crcs + num_bytes <= num_update_blocks) {
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                fram->write((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + received_par_crcs, crc_block, num_bytes);

                received_par_crcs += num_bytes;
            } else return throw_error(PARAMETER_MISMATCH);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::receive_block(unsigned char* data_block, uint16_t block_offset) {
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) > 0) {
            if((state_flags & PARTIAL_CRC_FLAG) == 0) {
                if(received_par_crcs < num_update_blocks * BLOCK_SIZE) {
                    if(block_offset < num_update_blocks) {
                        if(check_partial_crc(data_block, block_offset)) {
//                            slot_write_bytes(update_slot, block_offset * BLOCK_SIZE, data_block, BLOCK_SIZE);
                            serial.println("CRC matches for this block!");
                        } else {
                            if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) return;
                            return throw_error(CRC_MISMATCH);
                        }
                    } else return throw_error(OFFSET_OUT_OF_RANGE);
                } else return throw_error(PARAMETER_MISMATCH);
            } else return throw_error(PARTIAL_ALREADY_RECEIVED);
        } else return throw_error(METADATA_NOT_RECEIVED);
    } else return throw_error(UPDATE_NOT_STARTED);
}

bool SoftwareUpdateService::check_partial_crc(unsigned char* data_block, uint16_t block_offset) {
    unsigned char val = 0;

    for(int i = 0; i < BLOCK_SIZE; i++) {
        val = CRC_TABLE[val ^ data_block[i]];
    }

    unsigned char crc;
    if(!fram->ping()) {
        throw_error(NO_FRAM_ACCESS);
        return false;
    }
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + METADATA_SIZE + block_offset, &crc, 1);

    return crc == val;
}

void SoftwareUpdateService::check_md5(unsigned char slot_number) {
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    unsigned char digest[CRC_SIZE];

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (unsigned char*)&num_blocks, sizeof(uint16_t));

    unsigned char meta_crc[CRC_SIZE];
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE);

    unsigned char buffer[BLOCK_SIZE];

    for(int i = 0; i < num_blocks; i++) {
//        if((error = slot_read_bytes(slot_number, num_blocks * i, buffer, BLOCK_SIZE);
        MD5_Update(&md5_c, buffer, BLOCK_SIZE);
    }

    MD5_Final(digest, &md5_c);

    bool equal = true;
    for(int i = 0; i < CRC_SIZE; i++) {
        if(digest[i] != meta_crc[i]) {
            equal = false;
            break;
        }
    }

    payload_data[COMMAND_DATA] = equal;
}

void SoftwareUpdateService::stop_OTA() {
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    if((state_flags & UPDATE_FLAG) > 0) {
        state_flags &= ~UPDATE_FLAG;
        update_slot = 0;
        check_md5(update_slot);
        if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) return throw_error(payload_data[COMMAND_DATA]);
        if(payload_data[COMMAND_DATA]) {
           unsigned char temp = FULL;
           if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
           fram->write((METADATA_SIZE + PAR_CRC_SIZE) * update_slot, &temp, 1);
        } else return throw_error(MD5_MISMATCH);
    } else return throw_error(UPDATE_NOT_STARTED);
}

void SoftwareUpdateService::erase_slot(unsigned char slot) {
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    if((state_flags & UPDATE_FLAG) == 0) {
       unsigned char empty[METADATA_SIZE] = { 0 };
       if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
       fram->write((METADATA_SIZE + PAR_CRC_SIZE) * slot, empty, METADATA_SIZE);
       state_flags &= ~ERASE_FLAG;
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void SoftwareUpdateService::print_response() {
    serial.println("Response data:");

    serial.print("Payload size: ");
    serial.println(payload_size, DEC);

    serial.print("Service: ");
    serial.println(payload_data[COMMAND_SERVICE], DEC);

    serial.print("payload_data type: ");
    switch (payload_data[COMMAND_RESPONSE])
    {
    case COMMAND_ERROR:
        serial.println("Error");
        break;
    case COMMAND_REQUEST:
        serial.println("Request");
        break;
    case COMMAND_REPLY:
        serial.println("Reply");
        break;
    default:
        break;
    }

    serial.print("Method: ");
    switch (payload_data[COMMAND_METHOD])
    {
    case START_OTA:
        serial.println("START_OTA");
        break;
    case RECEIVE_METADATA:
        serial.println("RECEIVE_METADATA");
        break;
    case SEND_METADATA:
        serial.println("SEND_METADATA");
        break;
    case RECEIVE_PARTIAL_CRCS:
        serial.println("RECEIVE_PARTIAL_CRCS");
        break;
    case SEND_MISSED_PARTIALS:
        serial.println("SEND_MISSED_PARTIALS");
        break;
    case RECEIVE_BLOCK:
        serial.println("RECEIVE_BLOCK");
        break;
    case CHECK_MD5:
        serial.println("CHECK_MD5");
        break;
    case STOP_OTA:
        serial.println("STOP_OTA");
        break;
    case ERASE_SLOT:
        serial.println("ERASE_SLOT");
        break;
    default:
        break;
    }

    if(payload_size > PAYLOAD_SIZE_OFFSET) {
        serial.print("Payload data: ");
        for(int i = 0; i < payload_size - PAYLOAD_SIZE_OFFSET; i++) {
            if(payload_data[i + COMMAND_DATA] < 0x10) serial.print(0, DEC);
            serial.print(payload_data[i + COMMAND_DATA], HEX);
            serial.print(" ");
        }
        serial.println("\n");
    } else {
        serial.print("\n");
    }
}

void SoftwareUpdateService::print_metadata(unsigned char* metadata) {
    serial.println("Metadata:");
    serial.print("\tSlot status: ");
    switch (metadata[0])
    {
        case EMPTY:
            serial.println("Emtpy");
            break;
        case PARTIAL:
            serial.println("Partial");
            break;
        case FULL:
            serial.println("Full");
            break;
        default:
            serial.println("Unknown slot status!");
            break;
    }
    serial.print("\tVersion: ");
    if(metadata[CRC_SIZE+1] < 0x10) serial.print("0");
    serial.print(metadata[CRC_SIZE+1], HEX);
    if(metadata[CRC_SIZE+2] < 0x10) serial.print("0");
    serial.print(metadata[CRC_SIZE+2], HEX);
    if(metadata[CRC_SIZE+3] < 0x10) serial.print("0");
    serial.print(metadata[CRC_SIZE+3], HEX);
    if(metadata[CRC_SIZE+4] < 0x10) serial.print("0");
    serial.println(metadata[CRC_SIZE+4], HEX);
    serial.print("\tNumber of blocks: ");
    serial.println(metadata[CRC_SIZE+6] << 8 | metadata[CRC_SIZE+5], DEC);
    serial.print("\tMD5 CRC: ");
    for(int i = 0; i < CRC_SIZE; i++) {
        if(metadata[i + 1] < 0x10) serial.print("0");
        serial.print(metadata[i + 1], HEX);
    }
    serial.println();
}

void SoftwareUpdateService::throw_error(unsigned char error) {
    serial.print("Error(");
    serial.print(error, DEC);
    serial.print("): ");

    switch (error)
    {
    case NO_FRAM_ACCESS:
        serial.println("No access to FRAM.");
        break;
    case NO_SLOT_ACCESS:
        serial.println("No access to requested slot.");
        break;
    case SLOT_OUT_OF_RANGE:
        serial.println("Requested slot is out of range.");
        break;
    case MEMORY_FULL:
        serial.println("The memory is full.");
        break;
    case PARAMETER_MISMATCH:
        serial.println("Invalid parameter size provided to function.");
        break;
    case UPDATE_NOT_STARTED:
        serial.println("The update has not started yet.");
        break;
    case UPDATE_ALREADY_STARTED:
        serial.println("The update has already been started.");
        break;
    case METADATA_ALREADY_RECEIVED:
        serial.println("The metadata has already been received.");
        break;
    case METADATA_NOT_RECEIVED:
        serial.println("The metadata has not been received yet.");
        break;
    case PARTIAL_ALREADY_RECEIVED:
        serial.println("The partial crcs have already been received.");
        break;
    case PARTIAL_NOT_RECEIVED:
        serial.println("The partial crcs have not been received yet.");
        break;
    case CRC_MISMATCH:
        serial.println("A partial crc mismatch has occurred.");
        break;
    case MD5_MISMATCH:
        serial.println("The md5 hash does not match.");
        break;
    case OFFSET_OUT_OF_RANGE:
        serial.println("The requested offset is out of range.");
        break;
    case SLOT_NOT_EMPTY:
        serial.println("The requested slot is not empty yet.");
        break;
    default:
        break;
    }

    payload_data[COMMAND_RESPONSE] = COMMAND_ERROR;
    payload_size = PAYLOAD_SIZE_OFFSET + 1;
    payload_data[COMMAND_DATA] = error;
}
