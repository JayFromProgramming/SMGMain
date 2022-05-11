//
// Created by Jay on 3/26/2022.
//

#include "tag_communicator.h"

//#include <utility>
#include "clone_preset_builder.h"

using namespace mt2;

event_handlers* handlers;
unsigned char preShot[2];

void IR_init(){
    handlers = new event_handlers();
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
bool shoot(){
    noInterrupts();
    return send(preShot, (unsigned int) 14);
    interrupts();
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

// Processes system commands and executes their assigned handler if one exists
void process_sys_command(const unsigned char command) {
    switch (command) {
        case ADMIN_KILL:
            if (handlers->on_admin_kill != nullptr) handlers->on_admin_kill();
            break;
        case PAUSE_UNPAUSE:
            if (handlers->on_pause_unpause != nullptr) handlers->on_pause_unpause();
            break;
        case START_GAME:
            if (handlers->on_start_game != nullptr) handlers->on_start_game();
            break;
        case RESTORE_DEFAULTS:
            if (handlers->on_restore_defaults != nullptr) handlers->on_restore_defaults();
            break;
        case RESPAWN:
            if (handlers->on_respawn != nullptr) handlers->on_respawn();
            break;
        case INIT_NEW_GAME:
            if (handlers->on_new_game != nullptr) handlers->on_new_game();
            break;
        case FULL_AMMO:
            if (handlers->on_full_ammo != nullptr) handlers->on_full_ammo();
            break;
        case END_GAME:
            if (handlers->on_end_game != nullptr) handlers->on_end_game();
            break;
        case RESET_CLOCK:
            if (handlers->on_reset_clock != nullptr) handlers->on_reset_clock();
            break;
        case INIT_PLAYER:
            if (handlers->on_init_player != nullptr) handlers->on_init_player();
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
            if (handlers->on_full_armor != nullptr) handlers->on_full_armor();
            break;
        case CLEAR_SCORES:
            if (handlers->on_clear_scores != nullptr) handlers->on_clear_scores();
            break;
        case TEST_SENSORS:
            if (handlers->on_test_sensors != nullptr) handlers->on_test_sensors();
            break;
        case STUN_PLAYER:
            if (handlers->on_stun != nullptr) handlers->on_stun();
            break;
        case DISARM_PLAYER:
            if (handlers->on_disarm_player != nullptr) handlers->on_disarm_player();
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
        if (handlers->on_hit != nullptr) handlers->on_hit(player_id, static_cast<teams>(team_id),
                                                          static_cast<damage_table>(damage));
//        Serial.printf("Received shot packet\n"
//                      "Player ID: %d\n"
//                      "Team ID: %d\n"
//                      "Damage: %d\n", player_id, team_id, damage);
    } else { // This is where system packets are processed
        switch (messageByte){
            case ADD_HEALTH:
                if (handlers->on_add_health != nullptr) handlers->on_add_health();
                break;
            case ADD_ROUNDS:
                if (handlers->on_add_rounds != nullptr) handlers->on_add_rounds();
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
                if (handlers->on_clip_pickup != nullptr) handlers->on_clip_pickup();
                break;
            case HEALTH_PICKUP:
                if (handlers->on_health_pickup != nullptr) handlers->on_health_pickup();
                break;
            case FLAG_PICKUP:
                if (handlers->on_flag_pickup != nullptr) handlers->on_flag_pickup();
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

