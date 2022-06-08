//
// Created by Jay on 5/5/2022.
//

#ifndef SMGMAIN_RADIOINTERFACE_H
#define SMGMAIN_RADIOINTERFACE_H

#define RADIO_CHIP_SELECT_PIN 6
#define RADIO_CHIP_INTERRUPT_PIN 2

#define RADIO_FREQUENCY 915.0

#include "messages.h"
#include "mt2Library/mt2_protocol.h"

namespace wireless {

    enum class RadioStates {
        UNKNOWN, FAILED_INIT, INITIALIZED, RUNNING,
        OVERHEAT
    };

    String* getRadioStateString(RadioStates state);

    String stateStrings[] = {
            "UNKNOWN", "FAILED_INIT", "INITIALIZED", "RUNNING",
            "OVERHEAT"
    };

    typedef struct queuedMessage{
        MessageTypes type; // Type of message
        void* message; // Pointer to the message
        bool sent; // Whether the message has been sent

    } queuedMessage_t;

    // Wrapper for all different message types
    typedef struct transmissionReceipt{
        MessageTypes type; // Type of message
        void* message; // Pointer to the message
        elapsedMillis age;
        uint8_t ack_id;
        bool acked; // true if the message has been acknowledged
    } transmissionReceipt_t;

    struct radio_event_handlers {
        void (*on_fault)(RadioStates fault);
    };

     struct game_event_handlers {
        void (*on_clone_raw)(mt2::clone_t* clone) = nullptr;
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
        uint8_t device_id = 0;
        elapsedMillis last_controller_contact;
        DeviceTypes device_type = DeviceTypes::player_gun;
        game_event_handlers *event_handlers_ = nullptr;
        RadioStates state = RadioStates::UNKNOWN;

    public:
        void init(DeviceTypes type, uint8_t id);
        game_event_handlers* get_handlers();
        void check_for_data();
        bool sendEvent(GunEvents event, uint8_t data, uint8_t data2) const;
        transmissionReceipt_t last_transmission;

        void update_loop();

        void radio_send(uint8_t *data, uint8_t len);

        void keep_alive_processor(keep_alive_message *message);
    };

} // radio

#endif //SMGMAIN_RADIOINTERFACE_H
