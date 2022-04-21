//
// Created by Jay on 3/26/2022.
//

#include "tag_communicator.h"

#include <utility>
#include "clone_preset_builder.h"

using namespace mt2;

event_handlers* handlers;
unsigned char preShot[2];

void IR_init(){
    transmitter_init();
}

event_handlers* get_handlers(){
    return handlers;
}

void signalScan(){
    if(IRScan()){
        decodeMT2Data(get_buffer());
    }
}

void sendCommand(system_commands command){
    unsigned char command_data[2];
    command_data[0] = SYSTEM_COMMAND;
    command_data[1] = command;
//    command_data[2] = TERMINATION_LITERAL;
    send(command_data, (unsigned short) 2);
}

void sendShot(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg){
    unsigned char shot[2];
    shot[0] = B01111111 & playerID;
    shot[1] = teamID << 6 | dmg << 2;
    send(shot, (unsigned int) 14);
}

void buildShot(uint_least8_t playerID, uint_least8_t teamID, uint_least8_t dmg){
    preShot[0] = B01111111 & playerID;
    preShot[1] = teamID << 6 | dmg << 2;
}

// Use precalculated shot data to send shot instead of building it (disables interrupts)
void shoot(){
    cli(); // Disable interrupts
    send(preShot, (unsigned int) 14);
    sei(); // Enable interrupts
}

// Sends a clone object over IR
FLASHMEM void sendClone(clone *clone){
    unsigned char* array = build_clone_array(clone);
    send(array, (unsigned short) 40);
    delete array; // Array is no longer needed, release memory allocated to it
}

FLASHMEM void printClone(clone *clone){
    print_clone_values(clone);
}


void process_sys_command(const unsigned char command) {
    switch (command) {
        case ADMIN_KILL:
            if (handlers->on_admin_kill != nullptr) handlers->on_admin_kill();
            break;
        case PAUSE_UNPAUSE:
            break;
        case START_GAME:
            if (handlers->on_start_game != nullptr) handlers->on_start_game();
            break;
        case RESTORE_DEFAULTS:
            break;
        case RESPAWN:
            if (handlers->on_respawn != nullptr) handlers->on_respawn();
            break;
        case INIT_NEW_GAME:
            if (handlers->on_new_game != nullptr) handlers->on_new_game();
            break;
        case FULL_AMMO:
            break;
        case END_GAME:
            if (handlers->on_end_game != nullptr) handlers->on_end_game();
            break;
        case RESET_CLOCK:
            break;
        case INIT_PLAYER:
            break;
        case EXPLODE_PLAYER:
            if (handlers->on_explode != nullptr) handlers->on_explode();
            break;
        case NEW_GAME:
            if (handlers->on_new_game != nullptr) handlers->on_new_game();
            break;
        case FULL_HEALTH:
            if (handlers->on_full_health != nullptr) handlers->on_full_health();
            break;
        case FULL_ARMOR:
            break;
        case CLEAR_SCORES:
            break;
        case TEST_SENSORS:
            break;
        case STUN_PLAYER:
            if (handlers->on_stun != nullptr) handlers->on_stun();
            break;
        case DISARM_PLAYER:
            break;
        default:
            break;
    }
}

void decodeMT2Data(unsigned char* data){
    uint_least8_t messageByte = data[0];
    // Check if the highest (first) bit of the first message byte is a 0
    if (messageByte & 0x80) { // This is a shot packet
        uint_least8_t player_id = messageByte & B01111111;
        uint_least8_t team_id = (data[1] & B11000000) >> 6;
        uint_least8_t damage = (data[1] & B00111100) >> 2;
        if (handlers->on_hit != nullptr) handlers->on_hit(player_id, team_id, damage);
//        Serial.printf("Received shot packet\n"
//                      "Player ID: %d\n"
//                      "Team ID: %d\n"
//                      "Damage: %d\n", player_id, team_id, damage);
    } else { // This is where system packets are processed
        switch (messageByte){
            case ADD_HEALTH:

                break;
            case ADD_ROUNDS:

                break;
            case SYSTEM_DATA:
                switch(data[1]){
                    case CLONE: {
                        clone *new_clone = array_to_clone(data);
                        if (handlers->on_clone != nullptr) handlers->on_clone(new_clone);
                        delete new_clone;
                        break;
                    }
                    default:
                        break;
                }
            case SYSTEM_COMMAND:
                process_sys_command(data[1]);
                break;
            case CLIP_PICKUP:

                break;
            case HEALTH_PICKUP:

                break;
            case FLAG_PICKUP:

                break;
            default:

                break;
        }
    }
}


void set_game_flag(clone* preset, unsigned char flag, unsigned char val,
                   unsigned char flag_select){
    game_flag_setter(preset, flag, val, flag_select);
}

