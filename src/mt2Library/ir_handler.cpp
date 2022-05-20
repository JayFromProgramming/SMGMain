//
// Created by Jay on 3/25/2022.
//
//#include <Arduino.h>

#include "ir_handler.h"
#include "mt2_protocol.h"

#define IR_PIN 8
#define IR_FREQ 56000
#define IR_DUTY_CYCLE 128

#define RECEIVE_PIN 14

#define IR_RECEIVE_EOM_TIMEOUT 3400 // How long after an IR pulse to determine it's the end of the message

uint16_t transmission_buffer[1024];
IntervalTimer transmission_timer; // Timer for sending IR signals
volatile uint_fast16_t  transmission_position = 0; // Used to count where in the buffer we are
volatile uint_fast16_t  transmission_length;
bool transmission_in_progress = false; // Used to determine if we are currently sending IR signals
void (*on_transmission_complete)() = nullptr;

uint_fast16_t received_pulse_buffer[1024];
volatile uint_fast16_t  received_pulse_position = 0; // Used to count where in the buffer we are
elapsedMicros received_pulse_timer; // Timer for receiving IR signals

uint8_t received_bytes[1024];
uint16_t received_length = 0;

void transmit_start();
void receive_pulse();


FLASHMEM void transmitter_init() {
    pinMode(IR_PIN, OUTPUT);
    transmission_position = 0;
    transmission_length = 0;
    transmission_in_progress = false;
}

void receiver_attach(){
    // Attach interrupts to the reception pin for falling and rising edges of the signal pulses
    attachInterrupt(RECEIVE_PIN, receive_pulse, CHANGE);
}

void receiver_detach(){
    detachInterrupt(RECEIVE_PIN);
}

FLASHMEM void receiver_init() {
    pinMode(RECEIVE_PIN, INPUT);
    receiver_attach();
}

/*
 *  This function is used to decode the IR signal
 */

FASTRUN bool decode_pulse_duration(uint16_t total_pulses) {
//    Serial.println("Decoding possible MT2 signal"); // decodePulseWidthData might work if this doesn't

    int header_mark = 0;
    for (int i = 0; i < total_pulses; i++){ // Look for header
       uint16_t time = received_pulse_buffer[i];
       if (time < MT2_HEADER_LENGTH + MT2_TOLERANCE && time > MT2_HEADER_LENGTH - MT2_TOLERANCE) {
           header_mark = i;
           break;
       }
    }

    if (header_mark == 0) {
//        Serial.println("No header found - not MT2 signal");
        return false;
    }

    if (received_pulse_buffer[header_mark + 1] < MT2_SPACE_LENGTH + MT2_TOLERANCE && received_pulse_buffer[header_mark + 1] >
    MT2_SPACE_LENGTH - MT2_TOLERANCE) {
//        Serial.println("Valid space after header, continuing");
    } else {
        Serial.println("Invalid space after header, aborting");
        return false;
    }

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
//        Serial.println("Warning: ByteBuilder is not empty at the end of the message");
    }

    if (received_length == 0) {
//        Serial.println("No data found - not MT2 signal");
        return false;
    }

    return true;
}


/*
 * This function is used to encode the binary data into IR pulse timings for the IR transmitter.
 */
uint16_t encodeMT2(const unsigned char *data, unsigned int bits){
//    Serial.println("Calculating timings...");
    transmission_length = 2;
    transmission_buffer[0] = MT2_HEADER_LENGTH; // Header pulse is 2400us on
    transmission_buffer[1] = MT2_SPACE_LENGTH; // Inter-pulse gap is 600us off
    unsigned char byte_to_encode = '\0';
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

// This function is used to send the IR signal to the IR transmitter in bytes
bool send(const unsigned char *data, unsigned short bytes){
    return send(data, (unsigned int) bytes*8);
}


// Send data in a bit count
bool send(const unsigned char *data, unsigned int bits){
    // Flush the transmission buffer
    if (transmission_in_progress){
        return false;
    }
    for (unsigned short & i : transmission_buffer){
        i = 0;
    }
    int trans_length = encodeMT2(data, bits);
//    analogWriteResolution(11);
//    analogWriteFrequency(IR_PIN, MT2_FREQUENCY);
    unsigned long expected_trans_time = 0;
    for (int i = 0; i < trans_length; i++){
//        if (!(i % 2)) Serial.printf("%d ", transmission_buffer[i]);
        expected_trans_time += transmission_buffer[i];
    }
//    Serial.printf("\nExpected time in microseconds %d\n", expected_trans_time);
//    Serial.printf("Total length is %d\n", transmission_length);
//    irsend.sendRaw(transmission_buffer, transmission_length,
//                   56);
    transmit_start();
    return true;
}

void transmit_finished(){
    analogWrite(IR_PIN, 0);
    transmission_in_progress = false;
    if (on_transmission_complete != nullptr) on_transmission_complete();
}

// Custom IR transmitter method using interval timer
FASTRUN void transmit_method(){
    noInterrupts(); // Prevent interrupts while we are changing output state of the IR transmitter
    if (transmission_position % 2){
        // Connect the IR pin to the PWM generator
        tone(IR_PIN, IR_FREQ);
    } else {
        noTone(IR_PIN);
    }
    if (transmission_position >= transmission_length){
        transmission_timer.end();
        transmit_finished();
    }
    transmission_timer.update(transmission_buffer[transmission_position]);
    transmission_position++;
    interrupts(); // Re-enable interrupts
}

void transmit_start(){
    // Set IR_PIN to generate a PWM frequency modulated to IR_FREQ
//    analogWriteFrequency(IR_PIN, IR_FREQ * 9);
//    analogWriteResolution(8);
//    analogWrite(IR_PIN, IR_DUTY_CYCLE);
    tone(IR_PIN, IR_FREQ);
    // Set the IR_PIN to output a high voltage
    transmission_timer.priority(1); // Set the timer to the highest priority
    transmission_in_progress = true;
    transmission_position = 1;
    transmission_timer.begin(transmit_method, transmission_buffer[0]);
}

void transmitter_test(){
    // Send a 56khz carrier signal for 5ms

    // Flush the transmission buffer
    for (unsigned short & i : transmission_buffer){
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
FASTRUN void receive_pulse(){
    noInterrupts() // Prevent interrupts while we are logging the pulse
    if(!received_pulse_position){
        received_pulse_buffer[received_pulse_position] = received_pulse_timer; // Store the current timer value in the buffer
        received_pulse_position++; // Increment the position in the buffer
    } else {
        received_pulse_position = 0; // Reset the receive position to the start of the buffer
    }
    received_pulse_timer = 0; // Reset the timer to 0 (As the timer is always counting up)
    interrupts() // Re-enable interrupts
}

void flush_pulse_buffer(){
    for (unsigned short & i : received_pulse_buffer){
        i = 0;
    }
    received_pulse_position = 0;
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
    if (received_pulse_timer > IR_RECEIVE_EOM_TIMEOUT && received_pulse_position > 0){
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