//
// Created by Jay on 4/20/2022.
//

#ifndef SMGMAIN_LCDDRIVER_H
#define SMGMAIN_LCDDRIVER_H


#include "tagger.h"

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))

namespace display {

    struct menu_option_item {
        String** option_names = nullptr;
        uint16_t num_options = 0; // The number of options for the menu
        uint16_t selected_option = 0; // The index of the selected option
        bool is_active = false; // Whether the menu is active
    };

    struct menu_item {
        const char *name = nullptr;
        void (*func)() = nullptr;
        void (*func_param)(int) = nullptr;
        menu_option_item* sub_menu = nullptr;
        int func_arg = 0;
    };

    struct menu_holder {
        const char* name = nullptr;
        menu_item* items = nullptr;
        uint16_t num_items = 0;
        uint16_t selected_item = 0;
        uint16_t background_color = 0;
        uint16_t text_color = 0;
    };

    class lcdDriver {

    private:
        bool displaying_alert = false;
        tagger_state* game_state = nullptr;
        score_data* score = nullptr;

        uint16_t last_health = -1; //!< The last health value displayed on the LCD
        uint8_t last_ammo_count = -1; //!< The last ammo count displayed on the LCD
        uint8_t last_clip_count = -1; //!< The last clip count displayed on the LCD
        float last_angle = -90.0f; //!< The last angle of the progress circle displayed on the LCD
        uint8_t last_progress_time = -1; //!< The last progress time displayed on the LCD
        bool display_progress_circle = false; //!< Whether the progress circle is currently being displayed on the LCD
        elapsedMillis progress_circle_timer; //!< The timer for the progress circle
        uint32_t progress_circle_total_time = 0; //!< The total time for the progress circle

        bool backlight_on = true;

        // String holders
        char* clips_str = nullptr;
        char* ammo_str = nullptr;
        char* health_str = nullptr;
        char* reload_str = nullptr;
        char* old_progress_str = nullptr;
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

        void display_alert(String *title, String *info);

        void clear_alert();

        static void toggle_backlight();

        void clear_reload_str();

        void draw_menu();

        void load_and_display_menu(menu_holder* menu);

        static menu_holder* make_menu(const char* name);

        static menu_holder* make_menu(const char* name, uint16_t text_color, uint16_t background_color);

        static void add_menu_item(menu_holder* menu, const char* name, void (*func)());

        static void add_menu_item(menu_holder* menu, const char* name);

        static menu_option_item* add_submenu(menu_holder* menu, const char* name, void (*func)(int));

        static void add_submenu_item(menu_option_item* sub_menu, const char* name_new);

        void menu_increment();

        void menu_decrement();

        void menu_select(bool select);

        static void add_menu_item(menu_holder *menu, const char *name, void (*func)(int), int func_arg);

        void tagger_init_screen();

        static void add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step);

        static void submenu_set_selected(menu_option_item *menu, unsigned int selected);

        static void free_menu(menu_holder *menu);

        void load_free_display_menu(menu_holder *menu);

        static void
        add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step, const char *unit);

        void draw_system_elements();

        void progress_circle();

        static void clear_canvas();

        static void draw_canvas();

        static void add_submenu_values(menu_option_item *sub_menu, uint32_t start, unsigned int range, unsigned int step);

        static void
        add_submenu_values(menu_option_item *sub_menu, uint32_t start, uint32_t range, unsigned int step,
                           const char *unit);

        void progress_circle(float remaining_time, float total_time, String *header);

        void start_progress_circle(uint32_t total_milliseconds);

        void start_progress_circle(uint32_t total_milliseconds, String *header);

        void cancel_progress_circle();
    };

} // display

#endif //SMGMAIN_LCDDRIVER_H
