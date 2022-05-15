//
// Created by Jay on 3/25/2022.
//

#ifndef SMGMAIN_IR_HANDLER_H
#define SMGMAIN_IR_HANDLER_H

#include <cstdint>

uint8_t* get_buffer();

void transmitter_init();
void receiver_init();

bool send(const unsigned char *data, unsigned int bits);

bool send(const unsigned char *data, unsigned short bytes);

bool IR_available();

void transmitter_test();

#endif //SMGMAIN_IR_HANDLER_H
