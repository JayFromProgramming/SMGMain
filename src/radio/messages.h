//
// Created by Jay on 5/5/2022.
//

#ifndef SMGMAIN_MESSAGES_H
#define SMGMAIN_MESSAGES_H

#include <cstdint>

namespace wireless {

    enum MessageTypes : uint8_t {
        // Radio messages
        game_event = 0,
        game_command = 1,
        state_update_request = 2, // A request for a state update either containing a full update or a partial update
        full_state_update = 3, // A message containing the entire state of a device
        partial_state_update = 4, // A message containing a subset of the state of a device
        clone_transfer = 5,
        acknowledgement = 6, // Acknowledgement message
        keep_alive = 7, // A message to ask if the device is still alive
    };

    enum DeviceTypes : uint8_t {
        // Device types
        all_devices = 0xFF, // A message to all devices is a device type of 1111 1111
        not_defined_yet = 0x00, // A message to all devices is a device type of 0000 0000
        controller = 0,
        player_gun = 1,
        field_element = 2,
        field_lighting = 3,
    };

    enum Commands : uint8_t {
        // Admin commands from the game controller
        kill_player = 0,
        respawn_player = 1,
        stun_player = 2,
        set_full_health = 3,
        set_full_ammo = 4,
        set_full_armor = 5,
        add_ammo = 6,
        add_health = 7,
        add_armor = 8,
        clear_scores = 9
    };

    enum GameEvents : uint8_t {
        // Events from the game controller
        game_start = 0
    };

    enum GunEvents : uint8_t {
        // Events we can send to the game controller
        not_yet_defined = 0,
        killed = 1,
        respawned = 2,
        stunned = 3,
        jammed = 4,
        full_health = 5,
    };

    enum state_update_types : uint8_t {
        // The keys for state update values
    };

    struct gun_event_message { // Sent by a gun to the game controller to indicate a change of state event
        MessageTypes message_type = MessageTypes::game_event;
        DeviceTypes recipient_type = DeviceTypes::controller;
        DeviceTypes sender_type = DeviceTypes::player_gun;
        GunEvents event_type = GunEvents::not_yet_defined;
        uint8_t event_data_1 = 0;
        uint8_t event_data_2 = 0;
        uint8_t ack_id = 0;
    } __attribute__((packed));

    struct command_message {
        MessageTypes message_type = MessageTypes::game_command;
        DeviceTypes recipient_type = DeviceTypes::controller;
        DeviceTypes sender_type = DeviceTypes::player_gun;
    } __attribute__((packed));

    // When this message type is received an ack is sent back
    struct keep_alive_message {
        MessageTypes message_type = MessageTypes::keep_alive;
        DeviceTypes recipient_type = DeviceTypes::all_devices;
        DeviceTypes sender_type = DeviceTypes::not_defined_yet;
        uint8_t ack_id = 1; // The id of this message
    } __attribute__((packed));

    struct acknowledgement_message {
        MessageTypes message_type = MessageTypes::acknowledgement;
        DeviceTypes recipient_type = DeviceTypes::not_defined_yet; // Address values are initialized when the message is sent
        DeviceTypes sender_type = DeviceTypes::not_defined_yet;
        uint8_t ack_id = 0; // The id of the message that was acknowledged
    } __attribute__((packed));



}
#endif //SMGMAIN_MESSAGES_H
