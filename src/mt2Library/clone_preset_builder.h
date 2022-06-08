//
// Created by Jay on 3/26/2022.
//

#ifndef SMGMAIN_CLONE_PRESET_BUILDER_H
#define SMGMAIN_CLONE_PRESET_BUILDER_H

//#include <Arduino.h>
#include "mt2Library/mt2_protocol.h"

using namespace mt2;

void calcCheckSum(unsigned char *clone){
    unsigned char sum = 0;
    for(int i = 4; i < 38; i++){
        sum += clone[i];
    }
    clone[38] = sum % 0x100; // Modulo 256
}

int8_t confirmCheckSum(const unsigned char *clone) {
    unsigned char sum = 0;
    for (int i = 4; i < 38; i++) {
        sum += clone[i];
    }
    if (sum % 0x100 != clone[38]) {
        return -1;
    }
    return 1;
}

clone_t* array_to_clone(const unsigned char *data) {
    auto* c = new clone_t;
    c->team_id                  = (teams) data[6];
    c->clips_from_ammo_box      = data[8];
    c->health_from_medic_box    = data[9];
    c->hit_led_timout_seconds   = data[11];
    c->sound_set                = (sounds_set) data[12];
    c->overheat_limit           = data[13];
    c->damage_per_shot          = (damage_table) data[14];
    c->clip_size                = data[17];
    c->number_of_clips          = data[18];
    c->fire_selector            = (fire_mode) data[19];
    c->burst_size               = data[20];
    c->cyclic_rpm               = (fire_rate_table) data[21];
    c->reload_time              = data[22];
    c->ir_power                 = data[23];
    c->ir_range                 = (ir_range_table) data[24];
    c->tagger_bool_flags        = data[25];
    c->respawn_health           = (respawn_health) data[26];
    c->respawn_delay            = data[28];
    c->armour_value             = data[29];
    c->game_bool_flags_1        = data[30];
    c->game_bool_flags_2        = data[31];
    c->hit_delay                = (hit_delays) data[32];
    c->start_delay              = data[33];
    c->death_delay              = data[34];
    c->time_limit               = data[35];
    c->max_respawns             = data[36];
    c->checksum_valid           = confirmCheckSum(data);
    return c;
}


unsigned char * build_clone_array(clone_t* clone_preset){ // Copy the clone_t structure into the clone_t array
    auto* clone_array = new uint8_t [40];
    // Initialize the clone_t array
    for(int i = 0; i < 40; i++){
        clone_array[i] = 0;
    }
    clone_array[0]  = SYSTEM_DATA; // Start by indicating that this is a system data packet
    clone_array[1]  = CLONE; // Then indicate that this is a clone_t packet
    clone_array[2]  = TERMINATION_LITERAL; // Then indicate the start of the clone_t data
    clone_array[5]  = clone_preset->clone_slot_number;
    clone_array[6]  = clone_preset->team_id;
    clone_array[8]  = clone_preset->clips_from_ammo_box;
    clone_array[9]  = clone_preset->health_from_medic_box;
    clone_array[11] = clone_preset->hit_led_timout_seconds;
    clone_array[12] = clone_preset->sound_set;
    clone_array[13] = clone_preset->overheat_limit;
    clone_array[16] = clone_preset->damage_per_shot;
    clone_array[17] = clone_preset->clip_size;
    clone_array[18] = clone_preset->number_of_clips;
    clone_array[19] = clone_preset->fire_selector;
    clone_array[20] = clone_preset->burst_size;
    clone_array[21] = clone_preset->cyclic_rpm;
    clone_array[22] = clone_preset->reload_time;
    clone_array[23] = clone_preset->ir_power;
    clone_array[24] = clone_preset->ir_range;
    clone_array[25] = clone_preset->tagger_bool_flags;
    clone_array[26] = clone_preset->respawn_health;
    clone_array[28] = clone_preset->respawn_delay;
    clone_array[29] = clone_preset->armour_value;
    clone_array[30] = clone_preset->game_bool_flags_1;
    clone_array[31] = clone_preset->game_bool_flags_2;
    clone_array[32] = clone_preset->hit_delay;
    clone_array[33] = clone_preset->start_delay;
    clone_array[34] = clone_preset->death_delay;
    clone_array[35] = clone_preset->time_limit;
    clone_array[36] = clone_preset->max_respawns;
    clone_array[37] = 0xFF; // End of data
    calcCheckSum(clone_array);
    return clone_array;
}

FLASHMEM String* get_soundset_name(sounds_set set){
    switch (set) {
        case MIL_SIM:
            return (String *) "MIL-SIM";
        case SCI_FI:
            return (String *) "SCI-FI";
        case SILENCED:
            return (String *) "SILENCED";
        default:
            return (String *) "UNKNOWN";
    }
}

FLASHMEM String* get_team_name(teams team){
    switch(team){
        case TEAM_RED:
            return (String *) "Red";
        case TEAM_BLUE:
            return (String *) "Blue";
        case TEAM_YELLOW:
            return (String *) "Yellow";
        case TEAM_GREEN:
            return (String *) "Green";
        default:
            return (String *) "Unknown";
    }
}

FLASHMEM String* get_fire_rate(fire_rate_table rate){
    switch(rate){
        case RPM_250:
            return (String *) F("250 RPM");
        case RPM_300:
            return (String *) F("300 RPM");
        case RPM_350:
            return (String *) F("350 RPM");
        case RPM_400:
            return (String *) F("400 RPM");
        case RPM_450:
            return (String *) F("450 RPM");
        case RPM_500:
            return (String *) F("500 RPM");
        case RPM_550:
            return (String *) F("550 RPM");
        case RPM_600:
            return (String *) F("600 RPM");
        case RPM_650:
            return (String *) F("650 RPM");
        case RPM_700:
            return (String *) F("700 RPM");
        case RPM_750:
            return (String *) F("750 RPM");
        case RPM_800:
            return (String *) F("800 RPM");
    }
    return nullptr;
}


FLASHMEM String* get_damagetable_name(damage_table damage) {
    switch (damage) {
        case DAMAGE_1:
            return (String *) "1";
        case DAMAGE_2:
            return (String *) "2";
        case DAMAGE_4:
            return (String *) "4";
        case DAMAGE_5:
            return (String *) "5";
        case DAMAGE_7:
            return (String *) "7";
        case DAMAGE_10:
            return (String *) "10";
        case DAMAGE_15:
            return (String *) "15";
        case DAMAGE_17:
            return (String *) "17";
        case DAMAGE_20:
            return (String *) "20";
        case DAMAGE_25:
            return (String *) "25";
        case DAMAGE_30:
            return (String *) "30";
        case DAMAGE_35:
            return (String *) "35";
        case DAMAGE_40:
            return (String *) "40";
        case DAMAGE_50:
            return (String *) "50";
        case DAMAGE_75:
            return (String *) "75";
        case DAMAGE_100:
            return (String *) "100";
    }
    return (String *) "Unknown";
}

FLASHMEM String* get_fire_mode_string(fire_mode mode){
    switch(mode){
        case FIRE_MODE_SINGLE:
            return (String *) F("Single");
        case FIRE_MODE_BURST:
            return (String *) F("Burst");
        case FIRE_MODE_AUTO:
            return (String *) F("Full Auto");
        default:
            return (String *) F("Unknown");
    }
}

FLASHMEM String* get_health_string(respawn_health health){
    if (health >= HP_1 && health <= HP_20) { // Return raw value as its value as a string
        return new String('0' + health);
    }
    if (health >= HP_25 && health <= HP_200){ // Calculate 20 + ((health - 20) * 5)
        return new String('0' + (20 + ((health - 20) * 5)));
    }
    if (health >= HP_250 && health <= HP_999){ // Calculate 250 + ((health - 250) * 50)
        return new String('0' + (250 + ((health - 250) * 50)));
    }
    return (String *) F("Unknown");
}

FLASHMEM char* get_tagger_flags(unsigned char flags){
    char* buffer = static_cast<char *>(calloc(1, sizeof(char) * 32));
    buffer[0] = '\n';
    if (flags & 0x01)
        strcat(buffer, "\tMuzzle Flash: On\n");
    else
        strcat(buffer,"\tMuzzle Flash: Off\n");
    if (flags & 0x02)
        strcat(buffer,"\tOverheat: On");
    else
        strcat(buffer,"\tOverheat: Off");
    buffer[strlen(buffer)] = '\0';
    return buffer;
}

FLASHMEM char* get_game_flags(unsigned char flags1, unsigned char flags2){
    char* buffer = static_cast<char *>(calloc(1, sizeof(char) * 256));
    buffer[0] = '\n'; // Flags below are defined above in #define
    if (GAME_HIT_LED_ENABLE & flags1)
        strcat(buffer, "\tHit LED: Enabled\n");
    else
        strcat(buffer,"\tHit LED: Disabled\n");
    if (GAME_FRIENDLY_FIRE & flags1)
        strcat(buffer,"\tFriendly Fire: Enabled");
    else
        strcat(buffer,"\tFriendly Fire: Disabled");
    if (GAME_UNLIMITED_CLIPS & flags1)
        strcat(buffer,"\n\tUnlimited Clips: Enabled");
    else
        strcat(buffer,"\n\tUnlimited Clips: Disabled");
    if (GAME_ZOMBIE_MODE & flags1)
        strcat(buffer,"\n\tZombie Mode: Enabled");
    else
        strcat(buffer,"\n\tZombie Mode: Disabled");
    if (GAME_MEDIC_ENABLE & flags1)
        strcat(buffer,"\n\tMedic: Enabled");
    else
        strcat(buffer,"\n\tMedic: Disabled");
    if (GAME_GAMEBOX_RESET_ON_RESPAWN & flags1)
        strcat(buffer,"\n\tGamebox Reset on Respawn: Enabled");
    else
        strcat(buffer,"\n\tGamebox Reset on Respawn: Disabled");
    if (GAME_GAMEBOX_UNLIMITED_USE & flags1)
        strcat(buffer,"\n\tGamebox Unlimited Use: Enabled");
    else
        strcat(buffer,"\n\tGamebox Unlimited Use: Disabled");
    if (GAME_CTF_DISPLAY_ENABLE & flags2)
        strcat(buffer,"\n\tCTF Display: Enabled");
    else
        strcat(buffer,"\n\tCTF Display: Disabled");
    if (GAME_UNLIMITED_RESPAWN & flags2)
        strcat(buffer,"\n\tRespawns: Enabled");
    else
        strcat(buffer,"\n\tRespawns: Disabled");
    if (GAME_DISPLAY_NICKNAMES & flags2)
        strcat(buffer,"\n\tDisplay Nicknames: Enabled");
    else
        strcat(buffer,"\n\tDisplay Nicknames: Disabled");
    if (GAME_OLD_IR_LEVELS & flags2)
        strcat(buffer,"\n\tOld IR Levels: Enabled");
    else
        strcat(buffer,"\n\tOld IR Levels: Disabled");
    if (GAME_FULL_AMMO_ON_RESPAWN & flags2)
        strcat(buffer,"\n\tFull Ammo on Respawn: Enabled");
    else
        strcat(buffer,"\n\tFull Ammo on Respawn: Disabled");
    if (GAME_ENABLE_GAME_MODE & flags2)
        strcat(buffer,"\n\tGame Mode: Enabled (No idea what this does)");
    else
        strcat(buffer,"\n\tGame Mode: Disabled (No idea what this does)");
    buffer[strlen(buffer)] = '\0';
    return buffer;
}

void game_flag_setter(clone_t* preset, unsigned char flag, unsigned char val,
                      unsigned char flag_select){
    // Or the values of flags and flag
    if (flag_select == 0){
        if (val == 1){ // If val is 1, set the flag
            preset->game_bool_flags_1 |= flag;
        } else {
            preset->game_bool_flags_1 &= ~flag;
        }
    }
    else if (flag_select == 1){
        if (val == 1){
            preset->game_bool_flags_2 |= flag;
        } else {
            preset->game_bool_flags_2 &= ~flag;
        }
    }
}

FLASHMEM String* hitdelay_string(hit_delays delay){
    switch (delay) {
        case NO_DELAY:
            return (String *) F("No Delay");
        case ONE_QUARTER_SECOND:
            return (String *) F("1/4 Second");
        case HALF_SECOND:
            return (String *) F("1/2 Second");
        case THREE_QUARTER_SECOND:
            return (String *) F("3/4 Second");
        case ONE_SECOND:
            return (String *) F("1 Second");
        case TWO_SECONDS:
            return (String *) F("2 Seconds");
        case THREE_SECONDS:
            return (String *) F("3 Seconds");
        case FOUR_SECONDS:
            return (String *) F("4 Seconds");
        case FIVE_SECONDS:
            return (String *) F("5 Seconds");
        case SIX_SECONDS:
            return (String *) F("6 Seconds");
        case SEVEN_SECONDS:
            return (String *) F("7 Seconds");
        case EIGHT_SECONDS:
            return (String *) F("8 Seconds");
        case NINE_SECONDS:
            return (String *) F("9 Seconds");
        case TEN_SECONDS:
            return (String *) F("10 Seconds");
        case ELEVEN_SECONDS:
            return (String *) F("11 Seconds");
        case TWELVE_SECONDS:
            return (String *) F("12 Seconds");
        case THIRTEEN_SECONDS:
            return (String *) F("13 Seconds");
        case FOURTEEN_SECONDS:
            return (String *) F("14 Seconds");
        case FIFTEEN_SECONDS:
            return (String *) F("15 Seconds");
        case SIXTEEN_SECONDS:
            return (String *) F("16 Seconds");
        case SEVENTEEN_SECONDS:
            return (String *) F("17 Seconds");
        case EIGHTEEN_SECONDS:
            return (String *) F("18 Seconds");
        case NINETEEN_SECONDS:
            return (String *) F("19 Seconds");
        case TWENTY_SECONDS:
            return (String *) F("20 Seconds");
    }
    return (String *) F("Unknown");
}

void print_clone_values(clone_t* preset) {
    Serial.printf(F("Clone Values:\n"));
    Serial.printf(F("Name: %s\n"), preset->name);
    Serial.printf(F("Team ID: %s\n"), get_team_name(preset->team_id));
    Serial.printf(F("Clips from ammo box: %d\n"), preset->clips_from_ammo_box);
    Serial.printf(F("Health from medic box: %d\n"), preset->health_from_medic_box);
    Serial.printf(F("Hit LED timeout seconds: %d\n"), preset->hit_led_timout_seconds);
    Serial.printf(F("Sound set: %s\n"), get_soundset_name(preset->sound_set));
    Serial.printf(F("Overheat limit: %d\n"), preset->overheat_limit);
    Serial.printf(F("Damage per shot: %s\n"), get_damagetable_name(preset->damage_per_shot));
    Serial.printf(F("Clip size: %d\n"), preset->clip_size);
    Serial.printf(F("Number of clips: %d\n"), preset->number_of_clips);
    Serial.printf(F("Fire selector: %s\n"), get_fire_mode_string(preset->fire_selector));
    Serial.printf(F("Burst size: %d\n"), preset->burst_size);
    Serial.printf(F("Cyclic RPM: %s\n"), get_fire_rate(preset->cyclic_rpm));
    Serial.printf(F("Reload time: %d\n"), preset->reload_time);
    Serial.printf(F("IR power: %d\n"), preset->ir_power);
    Serial.printf(F("IR range: %d\n"), preset->ir_range);
    Serial.printf(F("Tagger flags: %s\n"), get_tagger_flags(preset->tagger_bool_flags));
    Serial.printf(F("Respawn health: %s\n"), get_health_string(preset->respawn_health));
    Serial.printf(F("Respawn delay: %d\n"), preset->respawn_delay);
    Serial.printf(F("Armour value: %d\n"), preset->armour_value);
    char* game_flags = get_game_flags(preset->game_bool_flags_1,
                                      preset->game_bool_flags_2);
    Serial.printf(F("Game flags: %s\n"), game_flags);
    free(game_flags);
    Serial.printf(F("Hit delay: %s\n"), hitdelay_string(preset->hit_delay));
    Serial.printf(F("Start delay: %d\n"), preset->start_delay);
    Serial.printf(F("Death delay: %d\n"), preset->death_delay);
    Serial.printf(F("Time limit: %d\n"), preset->time_limit);
    Serial.printf(F("Max respawns: %d\n"), preset->max_respawns);
}

#endif //SMGMAIN_CLONE_PRESET_BUILDER_H
