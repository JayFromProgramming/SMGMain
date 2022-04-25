//
// Created by Jay on 3/26/2022.
//

#ifndef SMGMAIN_MT2_PROTOCOL_H
#define SMGMAIN_MT2_PROTOCOL_H

#define MT2_HEADER_LENGTH 2400 // Length of the MT2 header in microseconds
#define MT2_ONE_LENGTH 1200 // Length of a 1 in microseconds
#define MT2_ZERO_LENGTH 600 // Length of a 0 in microseconds
#define MT2_SPACE_LENGTH 600 // Length of a space in microseconds
#define MT2_TOLERANCE 100 // Tolerance in microseconds +/-

#define MT2_FREQUENCY 56000 // Frequency of the MT2 protocol in Hz

#define SYSTEM_PLAYER 0x31 // Player ID for the system "Dozer"

// Game flags first byte
#define GAME_HIT_LED_ENABLE 0x02 // Game Flag to enable the hit LED
#define GAME_FRIENDLY_FIRE 0x04 // Game Flag to enable friendly fire
#define GAME_UNLIMITED_CLIPS 0x08 // Game Flag to enable unlimited clips
#define GAME_ZOMBIE_MODE 0x10 // Game Flag to enable zombie mode
#define GAME_MEDIC_ENABLE 0x20 // Game Flag to enable medic mode
#define GAME_GAMEBOX_RESET_ON_RESPAWN 0x40 // Game Flag to enable gamebox reset on respawn
#define GAME_GAMEBOX_UNLIMITED_USE 0x80 // Game Flag to enable gamebox unlimited use
// Game flags second byte
#define GAME_CTF_DISPLAY_ENABLE 0x04 // Game Flag to enable CTF display
#define GAME_RESPAWN_ENABLE 0x08 // Game Flag to enable respawn
#define GAME_DISPLAY_NICKNAMES 0x10 // Game Flag to enable display of nicknames
#define GAME_OLD_IR_LEVELS 0x20 // Game Flag to enable old IR levels
#define GAME_FULL_AMMO_ON_RESPAWN 0x40 // Game Flag to enable full ammo on respawn
#define GAME_ENABLE_GAME_MODE 0x80 // Game Flag to enable game mode

#define SYSTEM_PLAYER_ID 0x31 // Player ID for the system "Dozer"

#define GAME_ADMIN_ID 0xFF // Player ID for the system "Admin"

#include <Arduino.h>

namespace mt2 {

    enum teams {
        RED = 0x00,
        BLUE = 0x01,
        YELLOW = 0x02,
        GREEN = 0x03,
        NONE = 0x04
    };

    enum fire_mode {
        FIRE_MODE_SINGLE = 0x00,
        FIRE_MODE_BURST = 0x01,
        FIRE_MODE_AUTO = 0x02
    };

    enum sounds_set {
        MIL_SIM = 0x00,
        SCI_FI = 0x01,
        SILENCED = 0x02
    };

    enum respawn_health {
        HP_1 = 0x01, HP_2 = 0x02, HP_3 = 0x03,
        HP_4 = 0x04, HP_5 = 0x05, HP_6 = 0x06,
        HP_7 = 0x07, HP_8 = 0x08, HP_9 = 0x09,
        HP_10 = 0x0A, HP_11 = 0x0B, HP_12 = 0x0C,
        HP_13 = 0x0D, HP_14 = 0x0E, HP_15 = 0x0F,
        HP_16 = 0x10, HP_17 = 0x11, HP_18 = 0x12,
        HP_19 = 0x13, HP_20 = 0x14, HP_25 = 0x15,
        HP_30 = 0x16, HP_35 = 0x17, HP_40 = 0x18,
        HP_45 = 0x19, HP_50 = 0x1A, HP_55 = 0x1B,
        HP_60 = 0x1C, HP_65 = 0x1D, HP_70 = 0x1E,
        HP_75 = 0x1F, HP_80 = 0x20, HP_85 = 0x21,
        HP_90 = 0x22, HP_95 = 0x23, HP_100 = 0x24,
        HP_105 = 0x25, HP_110 = 0x26, HP_115 = 0x27,
        HP_120 = 0x28, HP_125 = 0x29, HP_130 = 0x2A,
        HP_135 = 0x2B, HP_140 = 0x2C, HP_145 = 0x2D,
        HP_150 = 0x2E, HP_155 = 0x2F, HP_160 = 0x30,
        HP_165 = 0x31, HP_170 = 0x32, HP_175 = 0x33,
        HP_180 = 0x34, HP_185 = 0x35, HP_190 = 0x36,
        HP_195 = 0x37, HP_200 = 0x38, HP_250 = 0x39,
        HP_300 = 0x3A, HP_350 = 0x3B, HP_400 = 0x3C,
        HP_450 = 0x3D, HP_500 = 0x3E, HP_550 = 0x3F,
        HP_600 = 0x40, HP_650 = 0x41, HP_700 = 0x42,
        HP_750 = 0x43, HP_800 = 0x44, HP_850 = 0x45,
        HP_900 = 0x46, HP_950 = 0x47, HP_999 = 0x48
    };

    enum damage_table {
        DAMAGE_1 = 0x00,
        DAMAGE_2 = 0x01,
        DAMAGE_4 = 0x02,
        DAMAGE_5 = 0x03,
        DAMAGE_7 = 0x04,
        DAMAGE_10 = 0x05,
        DAMAGE_15 = 0x06,
        DAMAGE_17 = 0x07,
        DAMAGE_20 = 0x08,
        DAMAGE_25 = 0x09,
        DAMAGE_30 = 0x0A,
        DAMAGE_35 = 0x0B,
        DAMAGE_40 = 0x0C,
        DAMAGE_50 = 0x0D,
        DAMAGE_75 = 0x0E,
        DAMAGE_100 = 0x0F
    };

    enum fire_rate_table {
        RPM_250 = 0x00,
        RPM_300 = 0x01,
        RPM_350 = 0x02,
        RPM_400 = 0x03,
        RPM_450 = 0x04,
        RPM_500 = 0x05,
        RPM_550 = 0x06,
        RPM_600 = 0x07,
        RPM_650 = 0x08,
        RPM_700 = 0x09,
        RPM_750 = 0x0A,
        RPM_800 = 0x0B
    };

    enum ir_range_table {
        MIN = 0x00,
        TEN_PERCENT = 0x01,
        TWENTY_PERCENT = 0x02,
        FORTY_PERCENT = 0x03,
        SIXTY_PERCENT = 0x04,
        EIGHTY_PERCENT = 0x05,
        MAX = 0x06
    };


    enum hit_delays {
        NO_DELAY = 0x00,
        ONE_QUARTER_SECOND = 0x01,
        HALF_SECOND = 0x02,
        THREE_QUARTER_SECOND = 0x03,
        ONE_SECOND = 0x04,
        TWO_SECONDS = 0x05,
        THREE_SECONDS = 0x06,
        FOUR_SECONDS = 0x07,
        FIVE_SECONDS = 0x08,
        SIX_SECONDS = 0x09,
        SEVEN_SECONDS = 0x0A,
        EIGHT_SECONDS = 0x0B,
        NINE_SECONDS = 0x0C,
        TEN_SECONDS = 0x0D,
        ELEVEN_SECONDS = 0x0E,
        TWELVE_SECONDS = 0x0F,
        THIRTEEN_SECONDS = 0x10,
        FOURTEEN_SECONDS = 0x11,
        FIFTEEN_SECONDS = 0x12,
        SIXTEEN_SECONDS = 0x13,
        SEVENTEEN_SECONDS = 0x14,
        EIGHTEEN_SECONDS = 0x15,
        NINETEEN_SECONDS = 0x16,
        TWENTY_SECONDS = 0x17
    };

    enum message_types {
        ADD_HEALTH = 0x80,
        ADD_ROUNDS = 0x81,
        SYSTEM_COMMAND = 0x83,
        SYSTEM_DATA = 0x87,
        CLIP_PICKUP = 0x8A,
        HEALTH_PICKUP = 0x8B,
        FLAG_PICKUP = 0x8C,
        TERMINATION_LITERAL = 0xE8,
    };

    enum system_data {
        CLONE = 0x01,
        SCORE_1 = 0x03,
        SCORE_2 = 0x04,
        SCORE_3 = 0x05,
    };

    enum system_commands {
        ADMIN_KILL = 0x00,
        PAUSE_UNPAUSE = 0x01,
        START_GAME = 0x02,
        RESTORE_DEFAULTS = 0x03,
        RESPAWN = 0x04,
        INIT_NEW_GAME = 0x05,
        FULL_AMMO = 0x06,
        END_GAME = 0x07,
        RESET_CLOCK = 0x08,
        INIT_PLAYER = 0x0a,
        EXPLODE_PLAYER = 0x0b,
        NEW_GAME = 0x0c,
        FULL_HEALTH = 0x0d,
        FULL_ARMOR = 0x0f,
        CLEAR_SCORES = 0x14,
        TEST_SENSORS = 0x15,
        STUN_PLAYER = 0x16,
        DISARM_PLAYER = 0x17,
    };

    typedef struct clone { // Clone Structure, with default values set
        char name[15] = "Default preset"; // Preset name used for display, max 14 characters
        teams team_id = NONE; // See section 2.3.1
        unsigned char clips_from_ammo_box = 0x00;
        unsigned char health_from_medic_box = 0x00;
        unsigned char hit_led_timout_seconds = 0xFF;
        sounds_set sound_set = MIL_SIM; // See section 2.3.2
        unsigned char overheat_limit = 0x00; // Rounds per minute
        damage_table damage_per_shot = DAMAGE_25; // See section 2.3.3
        unsigned char clip_size = 0x1E; // 0xFF is unlimited
        unsigned char number_of_clips = 0xCA; // 0xCA is unlimited
        fire_mode fire_selector = FIRE_MODE_SINGLE; // See section 2.3.4
        unsigned char burst_size = 0x05; // Number of shots per burst
        fire_rate_table cyclic_rpm = RPM_450; // See 2.3.5
        unsigned char reload_time = 0x03; // In seconds
        unsigned char ir_power = 0x00; // 0 is indoor, 1 is outdoor
        ir_range_table ir_range = MAX; // See section 2.3.7
        unsigned char tagger_bool_flags = B00000001; // See section 2.3.8
        respawn_health respawn_health = HP_100; // See section 2.3.9 (default is 100 [0x24])
        unsigned char respawn_delay = 0x00; // In ten second increments
        unsigned char armour_value = 0x00;
        unsigned char game_bool_flags_1 = B00001010; // See section 2.3.10
        unsigned char game_bool_flags_2 = B01001100; // See section 2.3.11
        hit_delays hit_delay = ONE_QUARTER_SECOND; // See section 2.3.12
        unsigned char start_delay = 0x00; // In seconds
        unsigned char death_delay = 0x00; // In seconds
        unsigned char time_limit = 0x00; // In minutes
        unsigned char max_respawns = 0x00;
        char checksum_valid = 0x00; // Flag to indicate this clone passed parity
        // -1 is invalid, 0 is unchecked, 1 is checked and valid
    } clone;

    short heath_lookup(respawn_health health);

    short hit_delay_to_micros(hit_delays delay);

    short damage_table_lookup(damage_table damage);

}

#endif //TLTPROJECT1_MT2_PROTOCOL_H
