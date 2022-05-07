//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_LCDDRIVER_H
#define SMGMAIN_LCDDRIVER_H


#include "tagger.h"

namespace display {

    class lcdDriver {

    private:
        tagger_state* game_state = nullptr;
        score_data* score = nullptr;
        unsigned short last_health = -1;
        unsigned char last_ammo_count = -1;
        unsigned char last_clip_count = -1;
        float last_angle = -90.0f;
        unsigned short last_reload_time = -1;
        bool already_reloading = false;

        bool backlight_on = true;

        // String holders
        char* clips_str = nullptr;
        char* ammo_str = nullptr;
        char* health_str = nullptr;
        char* reload_str = nullptr;
        char* old_reload_str = nullptr;
        char* death_str = nullptr;
        char* time_alive_str = nullptr;

        // Methods for selective updates of the display
        void draw_health_bar();

        void clear_ammo_count();
        void draw_ammo_count();

        void clear_clip_count();
        void draw_clip_count();

        void draw_death_screen();

        // Methods for drawing the entire display
        static void clear_screen();

        // Helper methods for drawing the screen
        float calc_health_percentage();

    public:

        static void displayInit();

        void pass_data_ptr(tagger_state *data, score_data *score_t);

        void update_hud(); // Runs the update checks for the display

        void clear(); // Clears the display

//        void override_text(String* text);

        static void toggle_backlight();

        void clear_reload_str();
    };

} // display

#endif //SMGMAIN_LCDDRIVER_H
