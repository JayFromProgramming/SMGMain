//
// Created by Jay on 5/5/2022.
//

#ifndef SMGMAIN_RADIOINTERFACE_H
#define SMGMAIN_RADIOINTERFACE_H

#define RADIO_CHIP_SELECT_PIN 6
#define RADIO_CHIP_INTERRUPT_PIN 2

#define RADIO_FREQUENCY 915.0

#include "messages.h"

namespace wireless {

    struct game_event_handlers {
        void (*on_clone_raw)(char *raw_clone[40]) = nullptr;
        void (*on_pause_unpause)() = nullptr;
        void (*on_respawn)() = nullptr;
        void (*on_full_health)() = nullptr;
        void (*on_full_ammo)() = nullptr;
        void (*on_admin_kill)() = nullptr;
        void (*on_start_game)() = nullptr;
        void (*on_new_game)() = nullptr;
        void (*on_end_game)() = nullptr;
        void (*on_stun)() = nullptr;
        void (*on_explode)() = nullptr;
        void (*on_clip_pickup)() = nullptr;
        void (*on_health_pickup)() = nullptr;
        void (*on_flag_pickup)() = nullptr;
        void (*on_add_health)() = nullptr;
        void (*on_add_rounds)() = nullptr;
        void (*on_restore_defaults)() = nullptr;
        void (*on_reset_clock)() = nullptr;
        void (*on_init_player)() = nullptr;
        void (*on_full_armor)() = nullptr;
        void (*on_clear_scores)() = nullptr;
        void (*on_test_sensors)() = nullptr;
        void (*on_disarm_player)() = nullptr;
    };


    class radioInterface {
    private:
        uint8_t device_id;
        DeviceTypes device_type;
        game_event_handlers *event_handlers_;
    public:
        void init(DeviceTypes type, uint8_t id);
        game_event_handlers* get_handlers();
        void check_for_data();
        void sendEvent(GunEvents event, uint8_t data, uint8_t data2) const;
    };

} // radio

#endif //SMGMAIN_RADIOINTERFACE_H
