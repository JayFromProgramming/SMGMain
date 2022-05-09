//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_LCDDRIVER_H
#define SMGMAIN_LCDDRIVER_H


#include "tagger.h"

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))

namespace display {

    struct menu_option_item {
        char **options; // An array of strings that are the options for the menu
        int num_options; // The number of options for the menu
        int selected_option; // The index of the selected option
    };

    struct menu_item {
        const char *name = nullptr;
        void (*func)() = nullptr;
        void (*func_param)(int) = nullptr;
        menu_option_item* options = nullptr;
        int func_arg = 0;
    };

    struct menu_holder {
        const char* name;
        menu_item* items;
        int num_items;
        int selected_item;
        uint16_t background_color;
        uint16_t text_color;
    };

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

        menu_holder* current_menu = nullptr;

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

        void draw_menu();

        void load_and_display_menu(menu_holder* menu);

        static menu_holder* make_menu(const char* name);

        static menu_holder* make_menu(const char* name, uint16_t text_color, uint16_t background_color);

        static void add_menu_item(menu_holder* menu, const char* name, void (*func)());

        static void add_menu_item(menu_holder* menu, const char* name);

        static menu_option_item* add_option_menu(menu_holder* menu, const char* name, void (*func)(int));

        static void add_option_menu_item(menu_option_item* menu, const char* name);

        void menu_increment();

        void menu_decrement();

        void menu_select(bool select);

        static void add_menu_item(menu_holder *menu, const char *name, void (*func)(int), int func_arg);

        void tagger_init_screen();

        void add_option_menu_values(menu_option_item *item, unsigned int range);

        static void add_option_menu_values(menu_option_item *item, unsigned int range, unsigned int step);

        static void option_menu_set_selected(menu_option_item *menu, unsigned int selected);
    };

} // display

#endif //SMGMAIN_LCDDRIVER_H
