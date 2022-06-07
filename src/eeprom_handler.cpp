//
// Created by Jay on 4/1/2022.
//

#include "eeprom_handler.h"

#include <EEPROM.h>

#define EEPROM_RESET_FLAG 0x1B // Used to indicate if the struct sizes have changed and the EEPROM needs to be reset

#define PRESET_START_ADDRESS 0x1F // EEPROM address where presets start ( 0x1F = 31 )
#define TOTAL_PRESETS 12

device_configs* device_config = nullptr;


/**
 * @brief A more compressed and optimized version of clone for storage in EEPROM
 */
typedef struct eeprom_preset {
    char name[14]{"Unnamed Clone"}; // Preset name used for display, max 14 characters
    teams team_id = mt2::TEAM_NONE; // Team ID
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

/**
 * @brief Calculates the position of a given preset in EEPROM
 * @param index - The index of the preset to calculate the position of
 * @return The EEPROM address of the preset
 */
FLASHMEM int calculate_preset_index(int index) {
    unsigned short addr = index * (int) sizeof(clone) + PRESET_START_ADDRESS;
    if (addr + sizeof(clone) > EEPROM.length()) {
        return -1;
    }
    return addr;
}

/**
 * Calculates the checksum of a preset
 * @param preset - The preset to calculate the checksum of
 * @return The checksum of the preset
 */
FLASHMEM int calculate_checksum(eeprom_preset *preset) {
    int parity = 0;
    for (int i = 0; i < sizeof(eeprom_preset); i++) {
        // Make sure we don't count the parity byte
        if (i != sizeof(eeprom_preset) - 1) {
            parity += (int) ((unsigned char *) preset)[i];
        }
    }
    return (parity % B111) << 5; // Parity is stored as ppp00000 where p is the parity
}

/**
 * @brief Checks if the checksum of a preset is valid
 * @param preset - The preset to check the checksum of
 * @return True if the checksum is valid, false otherwise
 */
FLASHMEM bool check_checksum(eeprom_preset *preset) {
    int parity = calculate_checksum(preset);
    return (char) (((preset->parity_hit_delay & B11100000) == parity) ? 1 : -1);
}

/**
 * @brief Converts a compressed EEPROM optimized preset to a full preset
 * @param raw - The raw EEPROM preset to convert
 * @return A pointer to the full preset
 * @warning The returned pointer must be freed by the caller
 * @warning The passed eeprom_preset* is freed by this method
 */
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
    preset->checksum_valid = check_checksum(raw);
    delete raw; // Free the eeprom_preset struct as it is no longer needed
    return preset;
}

/**
 * @brief Converts a full preset to a compressed EEPROM optimized preset
 * @param preset - The full preset to convert
 * @return A pointer to the compressed EEPROM preset
 * @warning The returned pointer must be freed by the caller
 */
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
    char parity = calculate_checksum(raw);
    raw->parity_hit_delay = parity | preset->hit_delay;
    return raw;
}

/**
 * @brief Loads a preset from EEPROM
 * @param preset_num - The preset number to load
 * @return A pointer to the loaded preset
 * @warning The returned pointer must be freed by the caller
 */
FLASHMEM clone* load_preset(uint8_t preset_num){
    auto* raw_preset = new eeprom_preset();
    EEPROM.get(calculate_preset_index(preset_num), *raw_preset);
    auto* preset = eeprom_to_preset(raw_preset);
    if (preset->checksum_valid == -1) {
//        delete preset;
        Serial.println("Preset is corrupted");
//        return nullptr;
    }
    return preset;
}

/**
 * @brief Saves a preset to EEPROM
 * @param preset_num - The preset number to save
 * @param preset* - A pointer to the preset to save
 */
FLASHMEM void save_preset(uint8_t preset_num, clone* preset){
    auto* raw_preset = preset_to_eeprom(preset);
    if (check_checksum(raw_preset) == 1){
        EEPROM.put(calculate_preset_index(preset_num), *raw_preset);
    }else{
        Serial.println("Parity calculation error");
    }
    EEPROM.put(calculate_preset_index(preset_num), *raw_preset);
    delete raw_preset;
}

/**
 * @brief Loads all available presets from EEPROM
 * @param length* - A pointer to an integer to store the length of the array
 * @return A pointer to an array of preset pointers
 * @warning The returned presets must be freed by the caller, and the array must be freed by the caller
 */
FLASHMEM clone** load_presets(int* length){
    auto* presets = new clone*[TOTAL_PRESETS];
    for (int i = 0; i < TOTAL_PRESETS; i++) {
        presets[i] = load_preset(i);
    }
    *length = TOTAL_PRESETS;
    return presets;
}

/**
 * @brief Sets all presets and system settings to their default values
 */
FLASHMEM void set_defaults(){ // Set all memory settings to default values and restart
    auto* default_preset = new clone();
    char name[15];
    sprintf(name, "Loaded Preset");
    save_preset(0, default_preset);
    for (int i = 1; i < TOTAL_PRESETS; i++) {
        sprintf(name, "Preset %2d", i);
        stpcpy(default_preset->name, name);
        save_preset(i, default_preset);
    }
    delete default_preset;

    device_config->boot_mode = 0;
    device_config->current_preset = 0;
    device_config->current_team = mt2::TEAM_RED;

    EEPROM.put(calculate_preset_index(1), *device_config);

    SCB_AIRCR = 0x05FA0004;
}

/**
 * @breif Calculates the remaining space in EEPROM
 * @return An integer representing the remaining space in EEPROM
 */
FLASHMEM int get_remaining_space(){
    int start = calculate_preset_index(0);
    int end = calculate_preset_index(TOTAL_PRESETS - 1);

    int used_space = end - start;
    return EEPROM.length() - used_space;
}

/**
 * @breif Returns the current boot mode
 * @return A byte representing the current boot mode
 */
FLASHMEM unsigned char get_boot_mode(){
    if (device_config->temp_boot_mode != 0xFF) {
        unsigned char boot_mode = device_config->temp_boot_mode;
        device_config->temp_boot_mode = 0xFF;
        EEPROM.put(1, *device_config);
        return boot_mode;
    } else {
        return device_config->boot_mode;
    }
}

/**
 * @brief Sets the current boot mode
 * @param boot_mode - A byte representing the new boot mode
 */
FLASHMEM void set_boot_mode(unsigned char mode) {
    device_config->boot_mode = mode;
    EEPROM.put(1, *device_config);
}

/**
 * @brief Sets a temporary boot mode to apply to the next boot cycle
 * @param mode - A byte representing the new boot mode
 */
FLASHMEM void set_temp_boot_mode(unsigned char mode) {
    device_config->temp_boot_mode = mode;
    EEPROM.put(1, *device_config);
}

/**
 * @brief Gets the current device ID
 * @return - A byte representing the current device ID
 */
FLASHMEM unsigned short get_device_id(){
    return device_config->device_id;
}

/**
 * @brief Sets the current device ID
 * @param id - A byte representing the new device ID
 */
FLASHMEM void set_device_id(unsigned short id) {
    device_config->device_id = id;
    EEPROM.put(1, *device_config);
}

/**
 * @brief Gets the current device configuration
 * @return A pointer to a device_config struct
 * @warning The returned device_config struct must be freed by the caller
 */
FLASHMEM device_configs* get_device_configs(){
    return device_config;
}
/**
 * @brief Sets the current device configuration
 * @param configs - A pointer to a device_config struct
 */
FLASHMEM void set_device_configs(device_configs* configs) {
    EEPROM.put(1, *configs);
}

/**
 * @brief Initializes the device EEPROM
 * @details On first boot or EEPROM format change, the EEPROM is initialized with default values
 * otherwise all values are preserved
 */
FLASHMEM void init_eeprom(){
    EEPROM.begin();
    if (EEPROM.read(0) != EEPROM_RESET_FLAG) { // If the first byte is not the correct flag set defaults
        EEPROM.write(0, EEPROM_RESET_FLAG);
        set_defaults();
    }

    device_config = new device_configs();
    EEPROM.get(1, *device_config);

    if (sizeof (device_configs) + 1 > PRESET_START_ADDRESS){
        #ifdef DEBUG
            Serial.println("EEPROM config collision");
        #endif

    }

    #ifdef DEBUG
    Serial.println("EEPROM initialized");
    #endif
}

