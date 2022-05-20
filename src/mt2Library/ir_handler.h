//
// Created by Jay on 3/25/2022.
//

#ifndef SMGMAIN_IR_HANDLER_H
#define SMGMAIN_IR_HANDLER_H

#include <cstdint>

uint8_t* get_buffer();

void transmitter_init();
void receiver_init();

bool send(const uint8_t *data, uint32_t bits);

bool send(const uint8_t *data, uint16_t bytes);

bool IR_available();

void transmitter_test();

#endif //SMGMAIN_IR_HANDLER_H
