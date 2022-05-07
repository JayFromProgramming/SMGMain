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
        unsigned short last_health = -1;
        unsigned char last_ammo_count = -1;
        unsigned char last_clip_count = -1;
        double last_angle = -1;
        bool already_reloading = false;

        // String holders
        char* clips_str = nullptr;
        char* ammo_str = nullptr;

        // Methods for selective updates of the display
        void draw_health_bar();

        void clear_ammo_count();
        void draw_ammo_count();

        void clear_clip_count();
        void draw_clip_count();

        // Methods for drawing the entire display
        static void clear_screen();

        // Helper methods for drawing the screen
        float calc_health_percentage();

    public:

        static void displayInit();

        void pass_data_ptr(tagger_state *data);

        void update_hud(); // Runs the update checks for the display

        void clear(); // Clears the display

//        void override_text(String* text);

    };

} // display

#endif //SMGMAIN_LCDDRIVER_H
