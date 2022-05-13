//
// Created by Jay on 5/5/2022.
//

#include "radioInterface.h"

#include <RadioHead.h>
#include <RH_RF69.h>

RH_RF69 radio(RADIO_CHIP_SELECT_PIN, RADIO_CHIP_INTERRUPT_PIN);

namespace wireless {

    void pass_sys_data(){

    }

    void radioInterface::init(DeviceTypes type, uint8_t id) {
        // Initialize the radio
        if (!radio.init()) {
            state = RadioStates::FAILED_INIT;
            return;
        }
        radio.setFrequency(RADIO_FREQUENCY);
        radio.setTxPower(14, true); // 14 dBm
        radio.setPromiscuous(true); // Tell the radio to receive any packet, regardless of destination
        radio.setThisAddress(id);
        radio.temperatureRead(); // Read the current temperature
        device_id = id;
        device_type = type;
        event_handlers_ = new game_event_handlers;
    }

    bool radioInterface::sendEvent(GunEvents event, uint8_t data, uint8_t data2) const {
        // Create a packet
        gun_event_message message;
        message.event_type = event;
        message.event_data_1 = data;
        message.event_data_2 = data2;
        radio.setHeaderFrom(device_id);
        radio.setModeTx();
        return radio.send((uint8_t *)&message, sizeof(gun_event_message));
    }

    game_event_handlers *radioInterface::get_handlers() {
        return event_handlers_;
    }


    void radioInterface::keep_alive_processor(keep_alive_message *message) {
        auto reply = new acknowledgement_message;
        reply->sender_type = this->device_type;
        reply->recipient_type = message->sender_type;

        radio_send((uint8_t *)reply, sizeof(acknowledgement_message));
    }

    void system_command_processor(command_message *message) {

    }

    void gun_event_processor(gun_event_message *message) {
        // Not implemented yet
    }

    void radioInterface::update_loop(){
        check_for_data();

    }

    void radioInterface::check_for_data() {
        // Check for data
        if (radio.available()) {
            // Get the data
            uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
            uint8_t len = 0;
            if (radio.recv(buf, &len)) {
                // Process the data
                if (len < sizeof(acknowledgement_message))
                    return;
                uint8_t message_type = buf[0];
                auto target_type = (DeviceTypes)buf[1];
                uint8_t target_id = radio.headerTo(); // The destination ID of the message
                if (target_type == device_type || target_type == DeviceTypes::all_devices) {
                    if (target_id == device_id || target_id == 0xFF) {
                        switch (message_type) {
                            case wireless::MessageTypes::game_event: {
                                // Load the event into a struct
                                auto *msg = (gun_event_message *) buf;
                                // Process the event
                                gun_event_processor(msg);
                            }
                            break;
                            case wireless::MessageTypes::game_command:{
                                // Load the event into a struct
                                auto *cmd = (command_message *) buf;
                                // Process the event
                                system_command_processor(cmd);
                            }
                            break;
                            case wireless::MessageTypes::keep_alive: {
                                // Load the event into a struct
                                auto *msg = (keep_alive_message *) buf;
                                // Process the event
                                keep_alive_processor(msg);
                            }
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    void radioInterface::radio_send(uint8_t *data, uint8_t len) {
        if(this->last_transmission.acked){

        }
        radio.send(data, len);
    }

} // radio