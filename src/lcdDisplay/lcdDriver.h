//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_LCDDRIVER_H
#define SMGMAIN_LCDDRIVER_H


#include "tagger.h"

namespace display {

    class lcdDriver {

    private:
        tagger_state* game_state;
        unsigned short last_health;
        unsigned char last_ammo_count;
        unsigned char last_clip_count;
        double last_angle;
        bool already_reloading;

        // String holders
        char* clips_str;
        char* ammo_str;

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

        void update_hud();

    };

} // display

#endif //SMGMAIN_LCDDRIVER_H
