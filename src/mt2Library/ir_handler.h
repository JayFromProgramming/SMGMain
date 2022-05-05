//
// Created by Jay on 3/25/2022.
//

#ifndef SMGMAIN_IR_HANDLER_H
#define SMGMAIN_IR_HANDLER_H

unsigned char * get_buffer();

void transmitter_init();
void receiver_init();

void send(const unsigned char *data, unsigned int bits);

void send(const unsigned char *data, unsigned short bytes);

char IRScan();

void transmitter_test();

#endif //TLTPROJECT1_IR_HANDLER_H
