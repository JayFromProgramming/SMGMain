//
// Created by Jay on 5/5/2022.
//

#include "radioInterface.h"

#include <RadioHead.h>
#include <RH_RF69.h>

RH_RF69 radio(RADIO_CHIP_SELECT_PIN, RADIO_CHIP_INTERRUPT_PIN);

namespace wireless {

    void radioInterface::init(DeviceTypes type, uint8_t id) {
        // Initialize the radio
        radio.init();
        radio.setFrequency(RADIO_FREQUENCY);
        radio.setTxPower(20, true);
        device_id = id;
        device_type = type;
        event_handlers_ = new game_event_handlers;
    }

    void radioInterface::sendEvent(GunEvents event, uint8_t data, uint8_t data2) const {
        // Create a packet
        gun_event_message message;
        message.sender_id = device_id;
        message.event_type = event;
        message.event_data_1 = data;
        message.event_data_2 = data2;
        radio.send((uint8_t *)&message, sizeof(gun_event_message));
    }

    game_event_handlers *radioInterface::get_handlers() {
        return event_handlers_;
    }

    void system_command_processor(command_message *message) {

    }

    void gun_event_processor(gun_event_message *message) {
        // Not implemented yet
    }

    void radioInterface::check_for_data() {
        // Check for data
        if (radio.available()) {
            // Get the data
            uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(buf);
            if (radio.recv(buf, &len)) {
                // Process the data
                uint8_t message_type = buf[0];
                auto target_type = (DeviceTypes)buf[1];
                uint8_t target_id = buf[2];
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
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

} // radio