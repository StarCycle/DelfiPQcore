/*
 * SoftwareUpdateService.cpp
 *
 *  Created on: 27 Jul 2019
 *      Author: Jasper Haenen
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

bool SoftwareUpdateService::process(DataMessage &command, DataMessage &workingBuffer) {
    if (command.getPayload()[COMMAND_SERVICE] == SOFTWAREUPDATE_SERVICE) {
        // prepare response frame
//        workingBuffer.setDestination(command.getSource());
//        workingBuffer.setSource(interface.getAddress());
        workingBuffer.setSize(PAYLOAD_SIZE_OFFSET);
        workingBuffer.getPayload()[COMMAND_SERVICE] = SOFTWAREUPDATE_SERVICE;
        workingBuffer.getPayload()[COMMAND_RESPONSE] = COMMAND_REPLY;
        workingBuffer.getPayload()[COMMAND_METHOD] = command.getPayload()[COMMAND_METHOD];

        payload_data = workingBuffer.getPayload();
        payload_size = PAYLOAD_SIZE_OFFSET;

        if(command.getPayload()[COMMAND_METHOD] != ERASE_SLOT) state_flags &= ~ERASE_FLAG;

        switch (command.getPayload()[COMMAND_METHOD]) {
        case START_OTA:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                    start_OTA(command.getPayload()[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nOTA started!");

                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_METADATA:
            if(command.getSize() == METADATA_SIZE - 1 + PAYLOAD_SIZE_OFFSET) {
                receive_metadata(&(command.getPayload()[COMMAND_DATA]));
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nMetadata received!");

            } else throw_error(PARAMETER_MISMATCH);
            break;

        case SEND_METADATA:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET + 1) {
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
            if(command.getSize() <= BLOCK_SIZE + PAYLOAD_SIZE_OFFSET) {
                receive_partial_crcs(&(command.getPayload()[COMMAND_DATA]), command.getSize() - PAYLOAD_SIZE_OFFSET);
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nPartial crc block received!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case RECEIVE_BLOCK:
            if(command.getSize() <= BLOCK_SIZE + 2 + PAYLOAD_SIZE_OFFSET) {
                receive_block(&(command.getPayload()[COMMAND_DATA + 2]), command.getPayload()[COMMAND_DATA] | (command.getPayload()[COMMAND_DATA + 1] << 8));
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nBlock received!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case CHECK_MD5:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] == 1 || command.getPayload()[COMMAND_DATA] == 2) {
                    check_md5(command.getPayload()[COMMAND_DATA] - 1);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nMD5 is correct!");
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case STOP_OTA:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET) {
                stop_OTA();
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nOTA is stopped!");
            } else throw_error(PARAMETER_MISMATCH);
            break;

        case ERASE_SLOT:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET + 1) {
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
        case SET_BOOT_SLOT:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET + 1) {
                if(command.getPayload()[COMMAND_DATA] < 3) {
                    set_boot_slot(command.getPayload()[COMMAND_DATA], false);
                    if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) serial.println("\nSlot code executed successfully!");
                } else throw_error(SLOT_OUT_OF_RANGE);
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case GET_NUM_MISSED_BLOCKS:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET) {
                get_num_missed_blocks();
                if(payload_data[COMMAND_RESPONSE] != COMMAND_ERROR) {
                    serial.print("\nNumber of missed blocks: ");
                    serial.println(payload_data[COMMAND_DATA], DEC);
                }
            } else throw_error(PARAMETER_MISMATCH);
            break;
        case GET_MISSED_BLOCKS:
            if(command.getSize() == PAYLOAD_SIZE_OFFSET) {
                get_missed_blocks();
            } else throw_error(PARAMETER_MISMATCH);
            break;
        default:
            break;
        }

        workingBuffer.setSize(payload_size);

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
            state = PARTIAL;
            if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
            fram->write((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &state, 1);
            state_flags |= UPDATE_FLAG;
            state_flags &= ~(METADATA_FLAG | PARTIAL_CRC_FLAG | MD5_INCORRECT_FLAG);
            received_par_crcs = 0;
            update_slot = slot_number;
            for(int i = 0; i < [MAX_BLOCK_AMOUNT/INT_SIZE]; i++) blocks_received[i] = 0;
            missed_pointer = 0;
        } else if(state == PARTIAL) {
            if((state_flags & MD5_INCORRECT_FLAG) > 0 && update_slot == slot_number) {
                if((state_flags & METADATA_FLAG) > 0) {
                    state_flags |= UPDATE_FLAG;
                } else return throw_error(METADATA_NOT_RECEIVED);
            } else return throw_error(UPDATE_NOT_CURRENT_SESSION);
        } else return throw_error(SLOT_NOT_EMPTY);
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void SoftwareUpdateService::receive_metadata(unsigned char* metadata) {
    payload_size = PAYLOAD_SIZE_OFFSET;

    if((state_flags & UPDATE_FLAG) > 0) {
        if((state_flags & METADATA_FLAG) == 0) {
            unsigned short temp_num_blocks = metadata[NUM_BLOCKS_OFFSET - 1] | (metadata[NUM_BLOCKS_OFFSET] << 8);
            if(temp_num_blocks <= MAX_BLOCK_AMOUNT) {
                if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
                fram->write((METADATA_SIZE + PAR_CRC_SIZE) * update_slot + 1, metadata, METADATA_SIZE - 1);
                num_update_blocks = temp_num_blocks;
                state_flags |= METADATA_FLAG;
            } else return throw_error(UPDATE_TO_BIG);
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
                    if(block_offset <= num_update_blocks) {
                        if(check_partial_crc(data_block, block_offset)) {
                            unsigned int sector =  1 << (((block_offset * BLOCK_SIZE + update_slot * SLOT_SIZE)) / SECTOR_SIZE);
                            if(!MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, sector)) return throw_error(NO_SLOT_ACCESS);
                            if(!MAP_FlashCtl_programMemory(data_block, (void*)(BANK1_ADDRESS + update_slot * SLOT_SIZE + block_offset * BLOCK_SIZE), BLOCK_SIZE)) return throw_error(NO_SLOT_ACCESS);
                            if(!MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, sector)) return throw_error(NO_SLOT_ACCESS);
                            blocks_received[block_offset / INT_SIZE] |= 1 << (block_offset % INT_SIZE);
                        } else {
                            blocks_received[block_offset / INT_SIZE] &= ~(1 << (block_offset % INT_SIZE));
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

    unsigned char digest[MD5_SIZE];

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (unsigned char*)&num_blocks, sizeof(uint16_t));

    unsigned char meta_crc[MD5_SIZE];
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, MD5_SIZE);

    MD5_Update(&md5_c, (unsigned char*)(BANK1_ADDRESS + slot_number * SLOT_SIZE), num_blocks * BLOCK_SIZE);

    MD5_Final(digest, &md5_c);

    bool equal = true;
    for(int i = 0; i < MD5_SIZE; i++) {
        if(digest[i] != meta_crc[i]) {
            equal = false;
            break;
        }
    }

    if(equal) state_flags &= ~MD5_INCORRECT_FLAG;
    else state_flags |= MD5_INCORRECT_FLAG;
    payload_data[COMMAND_DATA] = equal;
}

void SoftwareUpdateService::stop_OTA() {
    payload_size = PAYLOAD_SIZE_OFFSET + 1;

    if((state_flags & UPDATE_FLAG) > 0) {
        state_flags &= ~UPDATE_FLAG;
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

       if(!MAP_FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, 0xFFFF << (16 * slot))) return throw_error(NO_SLOT_ACCESS);
       if(!MAP_FlashCtl_performMassErase()) return throw_error(NO_SLOT_ACCESS);
       if(!MAP_FlashCtl_protectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, 0xFFFF << (16 * slot))) return throw_error(NO_SLOT_ACCESS);

       state_flags &= ~(ERASE_FLAG | METADATA_FLAG);
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void SoftwareUpdateService::set_boot_slot(unsigned char slot, bool permenant) {
    if(permenant){

    }
    check_md5(slot);
    if(payload_data[COMMAND_RESPONSE] == COMMAND_ERROR) return throw_error(payload_data[COMMAND_DATA]);

    unsigned char state;
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * slot, &state, 1);

    if(state == FULL) {
        uint8_t boot2new = slot;
        uint8_t confirmJump = 1;
        fram->write(0x7FF0, &boot2new, 1);
        fram->write(0x7FF1, &confirmJump, 1);
        MAP_SysCtl_rebootDevice();
    } else return throw_error(SLOT_NOT_PROGRAMMED);
}

void SoftwareUpdateService::get_num_missed_blocks() {
    uint16_t* count = (uint16_t*)(&payload_data[COMMAND_DATA]);
    *count = 0;
    payload_size = PAYLOAD_SIZE_OFFSET + sizeof(uint16_t);


    unsigned char state;
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * update_slot, &state, 1);

    switch(state) {
        case FULL:
            serial.println("The requested slot is already fully programmed.");
            return;
        case EMPTY:
            return throw_error(UPDATE_NOT_STARTED);
        default:
            break;
    }

    if((state_flags & UPDATE_FLAG) == 0) {
        int size = (num_update_blocks / INT_SIZE) + (((num_update_blocks % INT_SIZE) > 0) ? 1 : 0);
        for(int i = 0; i < size; i++) {
            if(blocks_received[i] < 0xFFFFFFFF) {
                for(int j = 0; j < INT_SIZE; j++) {
                    if((i * INT_SIZE + j) == num_update_blocks) {
                        //memcpy(&payload_data[COMMAND_DATA], &count, sizeof(uint16_t));
                        return;
                    }
                    if((blocks_received[i] & (1 << j)) == 0) (*count)++;
                }
            }
        }
        //memcpy(&payload_data[COMMAND_DATA], &count, sizeof(uint16_t));
    } else return throw_error(UPDATE_ALREADY_STARTED);
}

void SoftwareUpdateService::get_missed_blocks() {
    unsigned char state;
    if(!fram->ping()) return throw_error(NO_FRAM_ACCESS);
    fram->read((METADATA_SIZE + PAR_CRC_SIZE) * update_slot, &state, 1);

    switch(state) {
        case FULL:
            serial.println("The requested slot is already fully programmed.");
            return;
        case EMPTY:
            return throw_error(UPDATE_NOT_STARTED);
        default:
            break;
    }

    if((state_flags & UPDATE_FLAG) == 0) {
        while(missed_pointer < num_update_blocks) {
            int index = missed_pointer / INT_SIZE;

            if(blocks_received[index] < 0xFFFFFFFF) {
                do {
                    if(missed_pointer == num_update_blocks) {
                        missed_pointer = 0;
                        return;
                    }
                    if((payload_size - PAYLOAD_SIZE_OFFSET) == BLOCK_SIZE) return;

                    if((blocks_received[index] & (1 << missed_pointer % INT_SIZE)) == 0) {
                        memcpy(&payload_data[COMMAND_DATA + (payload_size - PAYLOAD_SIZE_OFFSET)], &missed_pointer, sizeof(uint16_t));
                        payload_size += 2;
                        serial.print("Block ");
                        serial.print(missed_pointer, DEC);
                        serial.println(" has been missed.");
                    }

                    missed_pointer++;
                }
                while((missed_pointer % INT_SIZE) != 0);
            } else missed_pointer += INT_SIZE;
        }

        missed_pointer = 0;
    } else return throw_error(UPDATE_ALREADY_STARTED);
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
    if(metadata[MD5_SIZE+1] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+1], HEX);
    if(metadata[MD5_SIZE+2] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+2], HEX);
    if(metadata[MD5_SIZE+3] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+3], HEX);
    if(metadata[MD5_SIZE+4] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+4], HEX);
    if(metadata[MD5_SIZE+5] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+5], HEX);
    if(metadata[MD5_SIZE+6] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+6], HEX);
    if(metadata[MD5_SIZE+7] < 0x10) serial.print("0");
    serial.print(metadata[MD5_SIZE+7], HEX);
    if(metadata[MD5_SIZE+8] < 0x10) serial.print("0");
    serial.println(metadata[MD5_SIZE+8], HEX);
    serial.print("\tNumber of blocks: ");
    serial.println(metadata[MD5_SIZE+10] << 8 | metadata[MD5_SIZE+9], DEC);
    serial.print("\tMD5 CRC: ");
    for(int i = 0; i < MD5_SIZE; i++) {
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
        serial.println("The update is still in progress.");
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
    case UPDATE_TO_BIG:
        serial.println("The update is too big for the memory slot.");
        break;
    case SLOT_NOT_PROGRAMMED:
        serial.println("The requested slot is not (completely) programmed.");
    case UPDATE_NOT_CURRENT_SESSION:
        serial.println("The update cannot start, because the update is not from the current session. Erase the slot first and retry.");
    default:
        break;
    }

    payload_data[COMMAND_RESPONSE] = COMMAND_ERROR;
    payload_size = PAYLOAD_SIZE_OFFSET + 1;
    payload_data[COMMAND_DATA] = error;
}
