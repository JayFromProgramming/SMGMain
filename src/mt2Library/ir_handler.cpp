//
// Created by Jay on 3/25/2022.
//
//#include <Arduino.h>

#include "ir_handler.h"
#include "mt2_protocol.h"
#include <IRremote.h>

#define IR_PIN 3

uint16_t transmission_buffer[1024];
unsigned char received_bytes[1024];

IRsend irsend;
IRrecv irrecv(16);

int received_length;
int transmission_length;


unsigned char * get_buffer(){
    return received_bytes;
}


FLASHMEM void transmitter_init() {
//    pinMode(LED_BUILTIN, OUTPUT);
//    digitalWrite(LED_BUILTIN, HIGH);
    irsend.begin(IR_PIN, false);
    irrecv.enableIRIn();

}

FLASHMEM void receiver_init() {
    irrecv.enableIRIn();
}

/*
 *  This function is used to decode the IR signal
 */

FASTRUN void decodeRawSignal(const uint16_t *Signal, int SignalLength) {
//    Serial.println("Decoding possible MT2 signal"); // decodePulseWidthData might work if this doesn't

    int header_mark = 0;
    for (int i = 0; i < SignalLength; i++){ // Look for header
       uint16_t time = Signal[i];
       if (time < MT2_HEADER_LENGTH + MT2_TOLERANCE && time > MT2_HEADER_LENGTH - MT2_TOLERANCE) {
           header_mark = i;
           break;
       }
    }

    if (header_mark == 0) {
//        Serial.println("No header found - not MT2 signal");
        return;
    }

    if (Signal[header_mark + 1] < MT2_SPACE_LENGTH + MT2_TOLERANCE && Signal[header_mark + 1] >
    MT2_SPACE_LENGTH - MT2_TOLERANCE) {
//        Serial.println("Valid space after header, continuing");
    } else {
        Serial.println("Invalid space after header, aborting");
        return;
    }

    int received_bit = 0;
    received_length = 0;
    uint_least8_t byte_builder[8]; // Stores the bits of the current working byte, so they can be assembled into a byte
    uint_least8_t is_space = 0; // 0 for mark, 1 for space
    for (int i = header_mark + 2; i < SignalLength; i++){ // Start decoding message
        uint16_t time = Signal[i]; // Get time of current bit in microseconds
        if (is_space) { // If odd value, check for a space
            if (time < MT2_SPACE_LENGTH + MT2_TOLERANCE && time > MT2_SPACE_LENGTH - MT2_TOLERANCE) {
                is_space = 0; // If space, set is_space to 0
            } else {
//                Serial.println("Invalid space timing, aborting");
                return;
            }
        } else if (time < MT2_ZERO_LENGTH + MT2_TOLERANCE && time > MT2_ZERO_LENGTH - MT2_TOLERANCE) {
            byte_builder[received_bit] = 0; // Zero Mark
            received_bit++; // Increment received_bit
            is_space = 1;
        } else if (time < MT2_ONE_LENGTH + MT2_TOLERANCE && time > MT2_ONE_LENGTH - MT2_TOLERANCE) {
            byte_builder[received_bit] = 1; // One Mark
            received_bit++; // Increment received_bit
            is_space = 1;
        } else {
//            Serial.printf("Unknown bit detected at %d, timing %d\n", i, time);
            is_space = 1;
        }
        if (received_bit >= 8) { // If we have a full byte, add it to the received_bytes array
            received_bytes[received_length] = 0; // Clear the byte
            for (int j = 0; j < 8; j++) { // Add the bits to the byte
                received_bytes[received_length] += byte_builder[j] << (7 - j); // Shift the bits to the right
            }
//            Serial.printf("Decoded byte %d: %d\n", received_length, received_bytes[received_length]);
            received_length++; // Increment the length of the received_bytes array
            received_bit = 0; // Reset the bit counter
        }
    }

    if (received_bit != 0){
//        Serial.println("Warning: ByteBuilder is not empty at the end of the message");
    }

    if (received_length == 0) {
//        Serial.println("No data found - not MT2 signal");
        return;
    }

}



/*
 * This function is used to encode the binary data into IR pulse timings for the IR transmitter.
 */
int encodeMT2(const unsigned char *data, unsigned int bits){
//    Serial.println("Calculating timings...");
    transmission_length = 2;
    transmission_buffer[0] = MT2_HEADER_LENGTH; // Header pulse is 2400us on
    transmission_buffer[1] = MT2_SPACE_LENGTH; // Inter-pulse gap is 600us off
    unsigned char byte_to_encode = '\0';
    // Extract each bit from the data array and calculate the pulse timings
    for (int i = 0; i < bits; i++){
        if (i % 8 == 0){ // If we are at the start of a new byte, get the byte
            byte_to_encode = data[i / 8];
        }
        if (byte_to_encode & (1 << (7 - (i % 8)))){ // If the bit is 1, add a mark
            transmission_buffer[transmission_length] = MT2_ONE_LENGTH;
            transmission_buffer[transmission_length + 1] = MT2_SPACE_LENGTH;
        } else { // If the bit is 0, add a space
            transmission_buffer[transmission_length] = MT2_ZERO_LENGTH;
            transmission_buffer[transmission_length + 1] = MT2_SPACE_LENGTH;
        }
        transmission_length += 2;
    }
    return transmission_length - 1; // Return the length of the transmission_buffer minus last space
}

// This function is used to send the IR signal to the IR transmitter in bytes
void send(const unsigned char *data, unsigned short bytes){
    send(data, (unsigned int) bytes*8);
}

// Send data in a bit count
void send(const unsigned char *data, unsigned int bits){
    // Flush the transmission buffer
    for (unsigned short & i : transmission_buffer){
        i = 0;
    }
    int trans_length = encodeMT2(data, bits);
//    analogWriteResolution(11);
//    analogWriteFrequency(IR_PIN, MT2_FREQUENCY);
    unsigned long expected_trans_time = 0;
    for (int i = 0; i < trans_length; i++){
        expected_trans_time += transmission_buffer[i];
    }
    irsend.sendRaw(transmission_buffer, transmission_length,
                   56);
    // Compare the time it took to send the message to the time is should have taken
//    pinMode(IR_PIN, OUTPUT);
    digitalWrite(IR_PIN, HIGH);
}

FASTRUN char IRScan(){
    if (irrecv.decode()){
        IRData data = irrecv.decodedIRData;
        if (data.protocol == UNKNOWN){
//            Serial.println("Candidate protocol detected, attempting to decode.");
//            Serial.printf("Bit length: %d\n", data.numberOfBits);
            irparams_struct rawIRStruct = *data.rawDataPtr; // Get the raw IR data from the interrupt handler
            decodeRawSignal(rawIRStruct.rawbuf, sizeof rawIRStruct.rawbuf); // Decode the raw IR data
            return 1;
        } else {
//            Serial.println("Known protocol detected, aborting.");
        }
        irrecv.resume();
    } else {
        return 0;
    }
    return 0;
}