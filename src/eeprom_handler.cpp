//
// Created by Jay on 4/1/2022.
//

#include "eeprom_handler.h"

#include <EEPROM.h>

#define EEPROM_RESET_FLAG 0x0F // Used to indicate if the struct sizes have changed and the EEPROM needs to be reset

#define PRESET_START_ADDRESS 0x1F // EEPROM address where presets start ( 0x1F = 31 )
#define TOTAL_PRESETS 12

typedef struct device_configs {
    unsigned char boot_mode;

    unsigned char screen_orientation; // 0 for left, 1 for right, 2 for up, 3 for down
    unsigned char device_type; // Will be used by the radio library once it's implemented
    unsigned char device_id;

    unsigned char current_preset;
    teams current_team;

} device_configs;

device_configs* device_config = nullptr;

typedef struct eeprom_preset {
    char name[14]{"Unnamed Clone"}; // Preset name used for display, max 14 characters
    teams team_id = mt2::NONE; // Team ID
    unsigned char clips_from_ammo_box = 0x00;
    unsigned char health_from_medic_box = 0x00;
    unsigned char hit_led_timout_seconds = 0xFF;
    unsigned char soundset_fireselect_irrange = 0; // See section 2.3.2
    unsigned char overheat_limit = 0x00; // Rounds per minute
    unsigned char dps_rpm = 0; // Combination of damage and RPM
    unsigned char clip_size = 0x1E; // 0xFF is unlimited
    unsigned char number_of_clips = 0xCA; // 0xCA is unlimited
    unsigned char burst_size = 0x05; // Number of shots per burst
    unsigned char reload_time = 0x03; // In seconds
    unsigned char ir_power = 0x00; // 0 is indoor, 1 is outdoor
    unsigned char tagger_bool_flags = B00000001; // See section 2.3.8
    respawn_health respawn_health = HP_100; // See section 2.3.9 (default is 100 [0x24])
    unsigned char respawn_delay = 0x00; // In ten second increments
    unsigned char armour_value = 0x00;
    unsigned char game_bool_flags_1 = B00000001; // See section 2.3.10
    unsigned char game_bool_flags_2 = B00000001; // See section 2.3.11
    unsigned char start_delay = 0x00; // In seconds
    unsigned char death_delay = 0x00; // In seconds
    unsigned char time_limit = 0x00; // In minutes
    unsigned char max_respawns = 0x00;
    unsigned char parity_hit_delay = 0x00; // 3 bits of parity data plus the hit delay data
} eeprom_preset;

FLASHMEM int eepromPresetSize() {
    return sizeof(eeprom_preset);
}

FLASHMEM int calcAddress(int index) {
    unsigned short addr = index * (int) sizeof(clone) + PRESET_START_ADDRESS;
    if (addr + sizeof(clone) > EEPROM.length()) {
        return -1;
    }
    return addr;
}

FLASHMEM int calcParity(eeprom_preset *preset) {
    int parity = 0;
    for (int i = 0; i < sizeof(eeprom_preset); i++) {
        // Make sure we don't count the parity byte
        if (i != sizeof(eeprom_preset) - 1) {
            parity += (int) ((unsigned char *) preset)[i];
        }
    }
    return (parity % B111) << 5; // Parity is stored as ppp00000 where p is the parity
}

FLASHMEM char checkParity(eeprom_preset *preset) {
    int parity = calcParity(preset);
    return (char) (((preset->parity_hit_delay & B11100000) == parity) ? 1 : -1);
}

FLASHMEM clone* eeprom_to_preset(eeprom_preset* raw){
    auto* preset = new clone();
    stpcpy(preset->name, raw->name);
    preset->team_id = raw->team_id;
    preset->clips_from_ammo_box = raw->clips_from_ammo_box;
    preset->health_from_medic_box = raw->health_from_medic_box;
    preset->hit_led_timout_seconds = raw->hit_led_timout_seconds;
    preset->sound_set = static_cast<sounds_set>(
            raw->soundset_fireselect_irrange & 0x03);
    preset->ir_range = static_cast<ir_range_table>(
            raw->soundset_fireselect_irrange & 0x04 >> 2);
    preset->fire_selector = static_cast<fire_mode>(
            raw->soundset_fireselect_irrange & 0x18 >> 3);
    preset->damage_per_shot = static_cast<damage_table>(raw->dps_rpm & 0x0F);
    preset->cyclic_rpm = static_cast<fire_rate_table>((raw->dps_rpm & 0xF0) >> 4);
    preset->overheat_limit = raw->overheat_limit;
    preset->clip_size = raw->clip_size;
    preset->number_of_clips = raw->number_of_clips;
    preset->burst_size = raw->burst_size;
    preset->reload_time = raw->reload_time;
    preset->ir_power = raw->ir_power;
    preset->tagger_bool_flags = raw->tagger_bool_flags;
    preset->respawn_health = raw->respawn_health;
    preset->respawn_delay = raw->respawn_delay;
    preset->armour_value = raw->armour_value;
    preset->game_bool_flags_1 = raw->game_bool_flags_1;
    preset->game_bool_flags_2 = raw->game_bool_flags_2;
    preset->start_delay = raw->start_delay;
    preset->death_delay = raw->death_delay;
    preset->time_limit = raw->time_limit;
    preset->max_respawns = raw->max_respawns;
    preset->hit_delay = static_cast<hit_delays>(raw->parity_hit_delay & B00011111);
    preset->checksum_valid = checkParity(raw);
    delete raw; // Free the eeprom_preset struct as it is no longer needed
    return preset;
}


FLASHMEM eeprom_preset* preset_to_eeprom(clone* preset){
    auto* raw = new eeprom_preset();
    stpcpy(raw->name, preset->name);
    raw->team_id = preset->team_id;
    raw->clips_from_ammo_box = preset->clips_from_ammo_box;
    raw->health_from_medic_box = preset->health_from_medic_box;
    raw->hit_led_timout_seconds = preset->hit_led_timout_seconds;
    // Combine the soundset, ir range and fire selector into a single byte for storage
    raw->soundset_fireselect_irrange = preset->sound_set | // Format is: 0b000FFISS
            preset->ir_range << 2 | preset->fire_selector << 3;
    // Combine the damage and cyclic RPM into a single byte for storage
    raw->dps_rpm = preset->damage_per_shot | preset->cyclic_rpm << 4;
    raw->overheat_limit = preset->overheat_limit;
    raw->clip_size = preset->clip_size;
    raw->number_of_clips = preset->number_of_clips;
    raw->burst_size = preset->burst_size;
    raw->reload_time = preset->reload_time;
    raw->ir_power = preset->ir_power;
    raw->tagger_bool_flags = preset->tagger_bool_flags;
    raw->respawn_health = preset->respawn_health;
    raw->respawn_delay = preset->respawn_delay;
    raw->armour_value = preset->armour_value;
    raw->game_bool_flags_1 = preset->game_bool_flags_1;
    raw->game_bool_flags_2 = preset->game_bool_flags_2;
    raw->start_delay = preset->start_delay;
    raw->death_delay = preset->death_delay;
    raw->time_limit = preset->time_limit;
    raw->max_respawns = preset->max_respawns;
    char parity = calcParity(raw);
    raw->parity_hit_delay = parity | preset->hit_delay;
    return raw;
}


FLASHMEM clone* load_preset(uint8_t preset_num){
    auto* raw_preset = new eeprom_preset();
    EEPROM.get(calcAddress(preset_num), *raw_preset);
    auto* preset = eeprom_to_preset(raw_preset);
    if (preset->checksum_valid == -1) {
//        delete preset;
        Serial.println("Preset is corrupted");
//        return nullptr;
    }
    return preset;
}

FLASHMEM void save_preset(uint8_t preset_num, clone* preset){
    auto* raw_preset = preset_to_eeprom(preset);
    if (checkParity(raw_preset) == 1){
        EEPROM.put(calcAddress(preset_num), *raw_preset);
    }else{
        Serial.println("Parity calculation error");
    }
    EEPROM.put(calcAddress(preset_num), *raw_preset);
    delete raw_preset;
}


FLASHMEM clone** load_presets(int* length){
    auto* presets = new clone*[TOTAL_PRESETS];
    for (int i = 0; i < TOTAL_PRESETS; i++) {
        presets[i] = load_preset(i);
    }
    *length = TOTAL_PRESETS;
    return presets;
}

FLASHMEM void set_defaults(){ // Set all memory settings to default values and restart
    auto* default_preset = new clone();

    for (int i = 0; i < TOTAL_PRESETS; i++) {
        char name[15];
        sprintf(name, "Preset %2d", i + 1);
        stpcpy(default_preset->name, name);
        save_preset(i, default_preset);
    }
    delete default_preset;

    device_config->boot_mode = 0;
    device_config->current_preset = 0;
    device_config->current_team = mt2::RED;

    EEPROM.put(calcAddress(1), *device_config);

    SCB_AIRCR = 0x05FA0004;
}

FLASHMEM int get_remaining_space(){
    int start = calcAddress(0);
    int end = calcAddress(TOTAL_PRESETS - 1);

    int used_space = end - start;
    return EEPROM.length() - used_space;
}

FLASHMEM unsigned char get_boot_mode(){
    return device_config->boot_mode;
}

FLASHMEM void set_boot_mode(unsigned char mode) {
    device_config->boot_mode = mode;
    EEPROM.put(calcAddress(1), *device_config);
}

FLASHMEM void init_eeprom(){
    EEPROM.begin();
    if (EEPROM.read(calcAddress(0)) == EEPROM_RESET_FLAG) { // If the first byte is 0xFF,
        // then the EEPROM has not been initialized on this board
        set_defaults();
        EEPROM.write(calcAddress(0), EEPROM_RESET_FLAG);
    }

    device_config = new device_configs();
    EEPROM.get(calcAddress(1), *device_config);

    if (sizeof (device_configs) + 1 > PRESET_START_ADDRESS){
        #ifdef DEBUG
            Serial.println("EEPROM config collision");
        #endif

    }

    #ifdef DEBUG
    Serial.println("EEPROM initialized");
    #endif
}

