//
// Created by Jay on 3/25/2022.
//
//#include <Arduino.h>

#include "ir_handler.h"
#include "mt2_protocol.h"
#include <pinout.h>

#define IR_FREQ 56000
#define IR_DUTY_CYCLE 128

#define IR_RECEIVE_EOM_TIMEOUT  // How long after an IR pulse to determine it's the end of the message

uint_fast16_t transmission_buffer[1024];
IntervalTimer transmission_timer; // Timer for sending IR signals
volatile uint_fast16_t  transmission_position = 0; // Used to count where in the buffer we are
volatile uint_fast16_t  transmission_length;
bool transmission_in_progress = false; // Used to determine if we are currently sending IR signals
void (*on_transmission_complete)() = nullptr;

uint_fast16_t received_pulse_buffer[1024];
volatile int_fast16_t  received_pulse_position = -1; // Used to count where in the buffer we are
elapsedMicros received_pulse_timer; // Timer for receiving IR signals

uint8_t received_bytes[1024];
uint16_t received_length = 0;

void transmit_start();
void receive_pulse();


FLASHMEM void transmitter_init() {
    pinMode(MUZZLE_IR_FLASH, OUTPUT);
    transmission_position = 0;
    transmission_length = 0;
    transmission_in_progress = false;
    receiver_init();
}

void receiver_attach(){
    // Attach interrupts to the reception pin for falling and rising edges of the signal pulses
    attachInterrupt(IR_IN, receive_pulse, CHANGE);
}

void receiver_detach(){
    detachInterrupt(IR_IN);
}

FLASHMEM void receiver_init() {
    pinMode(IR_IN, INPUT);
    receiver_attach();
}

/**
 * @brief Decodes received IR pulses into valid mt2 data
 * @param total_pulses Total pulses received
 * @return True if a valid mt2 signal was received false if signal was invalid
 */
FASTRUN bool decode_pulse_duration(uint16_t total_pulses) {
//    Serial.println("Decoding possible MT2 signal"); // decodePulseWidthData might work if this doesn't

//    int header_mark = 0;
//    for (int i = 0; i < total_pulses; i++){ // Look for header
//       uint16_t time = received_pulse_buffer[i];
//       if (time < MT2_HEADER_LENGTH + MT2_TOLERANCE && time > MT2_HEADER_LENGTH - MT2_TOLERANCE) {
//           header_mark = i;
//           break;
//       }
//    }
//
//    if (header_mark == 0) {
//        Serial.println("No header found - not MT2 signal");
//        return false;
//    }

    // Print out the buffer for debugging
//    Serial.print("Received pulses:\n");
//    for (int i = 0; i < total_pulses; i++) {
//        Serial.print(received_pulse_buffer[i]);
//        Serial.print("\n");
//    }

    // Check if the first pulse is a header
    if (!(received_pulse_buffer[0] < MT2_HEADER_LENGTH + MT2_TOLERANCE &&
    received_pulse_buffer[0] > MT2_HEADER_LENGTH - MT2_TOLERANCE)) {
        Serial.println("First pulse is not a header - not MT2 signal");
        return false;
    }

    uint16_t header_mark = 0;
//    if (received_pulse_buffer[header_mark + 1] < MT2_SPACE_LENGTH + MT2_TOLERANCE && received_pulse_buffer[header_mark + 1] >
//    MT2_SPACE_LENGTH - MT2_TOLERANCE) {
//        Serial.println("Valid space after header, continuing");
//    } else {
//        Serial.println("Invalid space after header, aborting");
//        return false;
//    }

    uint32_t received_bit = 0;
    received_length = 0;
    uint_least8_t byte_builder[8]; // Stores the bits of the current working byte, so they can be assembled into a byte
    uint_least8_t is_space = 0; // 0 for mark, 1 for space
    for (int i = header_mark + 2; i < total_pulses; i++){ // Start decoding message
        uint16_t time = received_pulse_buffer[i]; // Get time of current bit in microseconds
        if (is_space) { // If odd value, check for a space
            if (time < MT2_SPACE_LENGTH + MT2_TOLERANCE && time > MT2_SPACE_LENGTH - MT2_TOLERANCE) {
                is_space = 0; // If space, set is_space to 0
            } else {
//                Serial.println("Invalid space timing, aborting");
                return false;
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
        received_bytes[received_length] = 0; // Clear the byte
        for (int j = 0; j < 8; j++) { // Add the bits to the byte
//            Serial.print(byte_builder[j]);
            received_bytes[received_length] += byte_builder[j] << (7 - j); // Shift the bits to the right
        }
//        Serial.printf("Decoded byte %d: %d\n", received_length, received_bytes[received_length]);
        received_length++; // Increment the length of the received_bytes array
        received_bit = 0; // Reset the bit counter
    }

    if (received_length == 0) {
//        Serial.println("No data found - not MT2 signal");
        return false;
    }

//    Serial.println("Decoded MT2 signal");
//    // Print out the bytes for debugging
//    Serial.print("Received bytes:\n");
//    for (int i = 0; i < received_length; i++) {
//        Serial.print(received_bytes[i]);
//        Serial.print("\n");
//    }

    return true;
}


/**
 * @brief Encodes data bytes into mt2 timed pulses
 * @param data An array of bytes to transmit
 * @param bits Total bits to transmit
 * @return Returns the total pulses to transmit
 */
uint16_t encodeMT2(const uint8_t *data, uint_fast32_t bits){
//    Serial.println("Calculating timings...");
    transmission_length = 2;
    transmission_buffer[0] = MT2_HEADER_LENGTH; // Header pulse is 2400us on
    transmission_buffer[1] = MT2_SPACE_LENGTH; // Inter-pulse gap is 600us off
    uint8_t byte_to_encode = 0; //!< A working byte that is being encoded
    // Extract each bit from the data array and calculate the pulse timings
    for (uint32_t i = 0; i < bits; i++){
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

/**
 * @brief Sends data over the IR transmitter
 * @param data An array of bytes to transmit
 * @param bytes Total bytes to send
 * @return True if transmission started, false if transmitter was busy
 * @note A returned value of true does not mean transmission was successful, only that it has started
 */
bool send(const uint8_t *data, uint16_t bytes){
    return send(data, (uint32_t) bytes*8);
}

/**
 * @brief Sends data over the IR transmitter
 * @param data An array of bytes to transmit
 * @param bits Total bits to send
 * @return True if transmission started, false if transmitter or the receiver was busy
 * @note A returned value of true does not mean transmission was successful, only that it has started
 */
bool send(const uint8_t *data, uint32_t bits){
    // Flush the transmission buffer
    if (transmission_in_progress || received_pulse_position != -1){
        return false; // If the transmitter is busy or the receiver is receiving, return false
    }
    encodeMT2(data, bits); // Encode the data into the transmission buffer
    transmit_start();
    return true;
}

void transmit_finished(){
    noTone(MUZZLE_IR_FLASH);
    transmission_in_progress = false;
    if (on_transmission_complete != nullptr) on_transmission_complete();
    receiver_attach(); // Reattach the receiver
}

// Custom IR transmitter method using interval timer
void transmit_method(){
    cli(); // Prevent interrupts while we are changing output state of the IR transmitter
    if (transmission_position % 2){
        // Connect the IR pin to the PWM generator
        tone(MUZZLE_IR_FLASH, IR_FREQ);
    } else {
        noTone(MUZZLE_IR_FLASH);
    }
    if (transmission_position >= transmission_length){
        transmission_timer.end();
        transmit_finished();
    }
    transmission_timer.update(transmission_buffer[transmission_position]);
    transmission_position++;
    sei(); // Re-enable interrupts
}

void transmit_start(){
    // Set IR_PIN to generate a PWM frequency modulated to IR_FREQ
    tone(MUZZLE_IR_FLASH, IR_FREQ);
    receiver_detach(); // Detach the IR receiver during transmission
    // Set the IR_PIN to output a high voltage
    transmission_timer.priority(1); // Set the timer to the highest priority
    transmission_in_progress = true;
    transmission_position = 1;
    transmission_timer.begin(transmit_method, transmission_buffer[0] / 2);
}

void transmitter_test(){
    // Send a 56khz carrier signal for 5ms

    // Flush the transmission buffer
    for (uint_fast16_t & i : transmission_buffer){
        i = 0;
    }

    transmission_buffer[0] = MT2_HEADER_LENGTH;
    transmission_buffer[1] = MT2_HEADER_LENGTH;
    transmission_buffer[2] = MT2_ONE_LENGTH;
    transmission_buffer[3] = MT2_ONE_LENGTH;
    transmission_buffer[4] = MT2_ZERO_LENGTH;
    transmission_buffer[5] = MT2_ZERO_LENGTH;
    transmission_buffer[6] = MT2_SPACE_LENGTH;
    transmission_buffer[7] = MT2_SPACE_LENGTH;
    transmission_length = 8;
    transmit_start();
}

// ---- IR Receiver ----

// Method to return a pointer to the received data buffer
uint8_t* get_buffer(){
    return received_bytes;
}

// This method is called when the IR receiver changes state
FASTRUN void receive_pulse() {
    cli() // Prevent interrupts while we are logging the pulse
    if (received_pulse_timer > 100){ // If the pulse is too short, ignore it
        if (received_pulse_position != -1) received_pulse_buffer[received_pulse_position] = received_pulse_timer;
        // Store the current timer value in the buffer
        received_pulse_position++; // Increment the position in the buffer
        received_pulse_timer = 0; // Reset the timer to 0 (As the timer is always counting up)
    }
    sei() // Re-enable interrupts
}

void flush_pulse_buffer(){
    for (uint_fast16_t & i : received_pulse_buffer){
        i = 0;
    }
    received_pulse_position = -1;
}

/*
 * This method returns true if the buffer contains a valid message
 * If the pulse buffer contains data it will be processed upon the calling of this method
 * If the pulse buffer is empty it will return false
 * If the contents of the pulse buffer are invalid it will return false and the buffer will be flushed
 * If the contents of the pulse buffer are valid it will return true
 *
 * @return true if the buffer contains a valid message
 */
bool IR_available(){
    if (((int) received_pulse_timer > 5000) && (received_pulse_position > 0)){
        Serial.printf("Received message with %d pulses, last pulse %d\n", received_pulse_position,
                      (int) received_pulse_timer);
        // We have received a message
        // Check if the message is valid

        receiver_detach(); // Detach the receiver interrupt, so we can process the message uninterrupted

        if (decode_pulse_duration(received_pulse_position)){
            flush_pulse_buffer();
            receiver_attach(); // Re-attach the receiver interrupt
            return true;
        } else {
            // The message is invalid
            flush_pulse_buffer();
            receiver_attach(); // Re-attach the receiver interrupt
            return false;
        }
    } else {
        return false;
    }
}