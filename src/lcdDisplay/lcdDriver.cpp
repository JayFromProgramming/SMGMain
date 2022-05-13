//
// Created by Jay on 4/20/2022.
//

//                lcd.drawLine(RELOAD_CENTER_X + cos(reload_angle) * RELOAD_INNER_RADIUS,
//                             RELOAD_CENTER_Y + sin(reload_angle) * RELOAD_INNER_RADIUS,
//                             RELOAD_CENTER_X + cos(reload_angle) * RELOAD_RADIUS,
//                             RELOAD_CENTER_Y + sin(reload_angle) * RELOAD_RADIUS,
//                             ST77XX_GREEN);

#include "lcdDriver.h"
#include "../../.pio/libdeps/teensylc/Adafruit GFX Library/Adafruit_GFX.h"

#include "Adafruit-ST7735-Library-master/Adafruit_ST7789.h"
//#include <Adafruit_GFX.h>
#include <SPI.h>
#include <gfxfont.h>

#define TFT_CS        10
#define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        14

#define BACKLIGHT_PIN 15

#define LCD_HEIGHT    240

#define HEALTH_BAR_START_X  7
#define HEALTH_BAR_START_Y  10
#define HEALTH_BAR_WIDTH    233
#define HEALTH_BAR_HEIGHT   10

#define CLIP_TEXT_START_X   69
#define CLIP_TEXT_START_Y   84
#define CLIP_TEXT_SIZE      4

#define AMMO_TEXT_START_X   69
#define AMMO_TEXT_START_Y   128
#define AMMO_TEXT_SIZE      14

#define RELOAD_CENTER_X      128
#define RELOAD_CENTER_Y      144
#define RELOAD_RADIUS        85
#define RELOAD_INNER_RADIUS  50
#define RELOAD_TEXT_X_OFFSET -35
#define RELOAD_TEXT_Y_OFFSET -20
#define RELOAD_TEXT_SIZE     4


#define DEATH_TEXT_START_X   24
#define DEATH_TEXT_START_Y   10
#define DEATH_TEXT_SIZE      3

#define MAX_MENU_ITEMS       32
#define MAX_OPTION_MENU_OPTIONS  300
#define MAX_OPTION_LENGTH   32

Adafruit_ST7789 lcd = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Define a canvas
GFXcanvas16 canvas(240, 240);

bool backlight = true;

namespace display {

    void lcdDriver::displayInit() {
        lcd.init(240, 240, SPI_MODE3);
        pinMode(BACKLIGHT_PIN, OUTPUT);
        digitalWriteFast(BACKLIGHT_PIN, HIGH);
        lcd.setRotation(2);
        lcd.fillScreen(ST77XX_RED);
        lcd.fillScreen(ST77XX_GREEN);
        lcd.fillScreen(ST77XX_BLUE);
        lcd.fillScreen(ST77XX_WHITE);
        lcd.fillScreen(ST77XX_BLACK);
        lcd.setTextSize(4);
        for (int i = 0; i < 12; i++) {
            lcd.print("You should not see this");
        }
//        digitalWriteFast(BACKLIGHT_PIN, LOW);
    }

    void lcdDriver::pass_data_ptr(tagger_state *data, score_data *score_t) {
        clips_str = static_cast<char *>(malloc(sizeof(char) * 10));
        ammo_str  = static_cast<char *>(malloc(sizeof(char) * 10));
        reload_str = static_cast<char *>(malloc(sizeof(char) * 10));
        old_reload_str = static_cast<char *>(malloc(sizeof(char) * 10));
        health_str = static_cast<char *>(malloc(sizeof(char) * 10));
        death_str = static_cast<char *>(malloc(sizeof(char) * 64));
        time_alive_str = static_cast<char *>(malloc(sizeof(char) * 14));
        this->game_state = data;
        this->score = score_t;
    }


    // Runs a hud update check to see if the hud needs to be updated if so it will update the hud
    // This method has a variable execution time depending on if an update is needed or not
    void lcdDriver::update_hud() {
        // Check if any game state values have changed to determine if we need to update the screen

        if (this->game_state->reloading) {
            float remaining_time = ((float) this->game_state->reload_time - (float) millis()) / 1000;
            this->progress_circle(remaining_time, (float) this->game_state->currentConfig->reload_time);
        } else {
            if (!this->game_state->started){
//                tagger_init_screen();
            }
            if (already_progressing) { // Stop reloading animation
                already_progressing = false;
                last_angle = -90.0f;
                lcdDriver::clear_screen();
                this->last_health = -1;
                this->last_clip_count = -1;
                this->last_ammo_count = -1;
            }
            if (this->game_state->health != this->last_health && this->last_health == 0) {
                clear_screen();
                this->draw_clip_count();
                this->draw_ammo_count();
            }
            if (this->game_state->health != this->last_health) {
                this->draw_health_bar();
                this->last_health = this->game_state->health;
            }
            if (this->game_state->clip_count != this->last_clip_count) {
                this->clear_clip_count();
                this->draw_clip_count();
                this->last_clip_count = this->game_state->clip_count;
            }
            if (this->game_state->ammo_count != this->last_ammo_count) {
                this->clear_ammo_count();
                this->draw_ammo_count();
                this->last_ammo_count = this->game_state->ammo_count;
            }
            this->draw_dynamic_elements();
        }
    }

    void lcdDriver::progress_circle(float remaining_time, float total_time) {
        if (this->already_progressing) {
            float remaining_reload_percent =  remaining_time / total_time;
            if (remaining_reload_percent > 1.0f) remaining_reload_percent = 1.0f;
            float remaining_angle = ((1 - remaining_reload_percent) * 360.0f) - 90.0f;
            // Sweep the circle with a line based on the remaining reload time to make a full circle
            // The line starts from the outer edge of the inner circle and ends at the outer edge of the outer circle
            for (float angle = last_angle; angle < remaining_angle; angle += 0.1f) {
                int x = RELOAD_CENTER_X + RELOAD_RADIUS * cos(angle * M_PI / 180);
                int y = RELOAD_CENTER_Y + RELOAD_RADIUS * sin(angle * M_PI / 180);
                int x2 = RELOAD_CENTER_X + RELOAD_INNER_RADIUS * cos(angle * M_PI / 180);
                int y2 = RELOAD_CENTER_Y + RELOAD_INNER_RADIUS * sin(angle * M_PI / 180);
                lcd.drawLine(x, y, x2, y2, ST77XX_GREEN);
            }
            last_angle = remaining_angle;

            // Print the remaining reload time in seconds
            if (remaining_time < 10) {
                sprintf(reload_str, "%.1f", remaining_time);
            } else if (remaining_time < 99) {
                sprintf(reload_str, "99");
            } else sprintf(reload_str, "%d", (unsigned short) remaining_time);

            last_reload_time = (unsigned short) remaining_time;

            if (strcmp(reload_str, old_reload_str) != 0) {
                lcd.setCursor(RELOAD_CENTER_X + RELOAD_TEXT_X_OFFSET, RELOAD_CENTER_Y + RELOAD_TEXT_Y_OFFSET);
                lcd.setTextSize(RELOAD_TEXT_SIZE);
                lcd.setTextColor(ST77XX_GREEN);
                this->clear_reload_str();
                lcd.print(reload_str);
                strcpy(old_reload_str, reload_str);
            }

        } else { // Start reloading animation
            already_progressing = true;
            this->clear_clip_count();
            this->clear_ammo_count();
            lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_RADIUS, ST77XX_YELLOW);
            lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_INNER_RADIUS, ST77XX_BLACK);
        }
    }

    float lcdDriver::calc_health_percentage() {
        return (float)game_state->health / (float) game_state->max_health;
    }

    void lcdDriver::clear_screen() {
        lcd.fillScreen(ST77XX_BLACK);
    }

    void lcdDriver::tagger_init_screen() {
        lcd.fillScreen(ST77XX_BLACK);
        lcd.setCursor(0, 0);
        lcd.setTextSize(4);
        lcd.setTextColor(ST77XX_GREEN);
        lcd.print("Hold Trigger to Start");
        lcd.setCursor(0, 12);

        lcd.setTextSize(2);
        // Print some info about current config
        lcd.print("Health: " + String(game_state->max_health) + " | " +
                  "Respawns: " + String(game_state->max_respawns) + "\n" +
                  "Clips: " + String(game_state->clip_count) + " | ");
    }

    // These are elements that are always changing
    void lcdDriver::draw_dynamic_elements() {
//        lcd.setCursor(0, 45);
//        lcd.setTextSize(1);
//        int16_t x, y;
//        uint16_t w, h;
//        String str = String(this->game_state->last_shot);
//        lcd.getTextBounds(str.c_str(), 0, 45, &x, &y, &w, &h);
//        lcd.fillRect(x, y, w, h, ST77XX_BLACK);
//        lcd.print(this->game_state->last_shot);
    }

    void lcdDriver::draw_death_screen() {
        clear_screen();
        lcd.fillScreen(ST77XX_RED);
        lcd.setCursor(DEATH_TEXT_START_X, DEATH_TEXT_START_Y);
        lcd.setTextSize(DEATH_TEXT_SIZE);
        lcd.setTextColor(ST77XX_WHITE);
        uint32_t time_alive = this->score->last_alive_time;
        uint32_t time_alive_seconds = time_alive / 1000;
        uint32_t time_alive_minutes = time_alive_seconds / 60;
        sprintf(time_alive_str, "%lu:%02lu", time_alive_minutes, time_alive_seconds % 60);
        String killer;
        if (this->score->killer_name != nullptr && this->score->assist_name != nullptr) {
            killer = String(*this->score->killer_name + " + " + *this->score->assist_name);
        } else if (this->score->killer_name != nullptr) {
            killer = String(*this->score->killer_name);
        } else
            killer = String("Unknown");

        sprintf(death_str, "GAME OVER!\n"
                           "Shots fired:\n%d\n"
                           "Time alive:\n%13s\n"
                           "Killed by:\n%s", score->rounds_fired_game, time_alive_str, killer.c_str());
        lcd.print(death_str);
    }

    void lcdDriver::draw_health_bar() {
        lcd.fillRect(0, 0, 128, 10, ST77XX_BLACK);
        lcd.setCursor(0, 0);
        lcd.setTextSize(1);
        lcd.setTextColor(ST77XX_WHITE);
        lcd.print("Health: ");
        lcd.print(this->game_state->health);
        lcd.print("/");
        lcd.print(this->game_state->max_health);
        if (this->game_state->health <= 0) {
            draw_death_screen();
        } else if (this->game_state->health == this->game_state->max_health) {
            lcd.fillRect(0, 10, 240, 20, ST77XX_GREEN);
        } else {
            lcd.fillRect((int)(calc_health_percentage() * 240), 10,
                         240, 20, ST77XX_RED);
            lcd.fillRect(0, 10,
                         (int)(calc_health_percentage() * 240), 20, ST77XX_GREEN);
        }
    }

    void lcdDriver::draw_ammo_count() {
        sprintf(ammo_str, "%02d", game_state->ammo_count);
        lcd.setCursor(AMMO_TEXT_START_X, AMMO_TEXT_START_Y);
        lcd.setTextSize(AMMO_TEXT_SIZE);
        if (game_state->ammo_count > 5) { // If ammo count is above 5 ammo count is green
            lcd.setTextColor(ST77XX_GREEN);
        } else if (game_state->ammo_count > 0) { // If ammo count is below 5 ammo count is yellow
            lcd.setTextColor(ST77XX_YELLOW);
        } else { // If player has no ammo ammo count is red
            lcd.setTextColor(ST77XX_RED);
        }

        lcd.print(ammo_str);
    }

    void lcdDriver::draw_clip_count() {
        lcd.setCursor(CLIP_TEXT_START_X, CLIP_TEXT_START_Y);
        lcd.setTextSize(CLIP_TEXT_SIZE);
        if (game_state->currentConfig->game_bool_flags_1 & GAME_UNLIMITED_CLIPS) {
            lcd.setTextColor(ST77XX_WHITE);
            sprintf(clips_str, "C%2dx99", game_state->currentConfig->clip_size);
        } else {
            if (game_state->clip_count > 5) { // If clip count is above 5 clip count is green
                lcd.setTextColor(ST77XX_GREEN);
            } else if (game_state->clip_count > 0) { // If clip count is below 5 clip count is yellow
                lcd.setTextColor(ST77XX_YELLOW);
            } else { // If player has no clips clip count is red
                lcd.setTextColor(ST77XX_RED);
            }
            sprintf(clips_str, "C%2dx%2d", game_state->currentConfig->clip_size, game_state->clip_count);
        }
        lcd.print(clips_str);
    }

    void lcdDriver::clear_ammo_count() {
        int16_t  x1, y1;
        uint16_t w, h;
        lcd.setTextSize(AMMO_TEXT_SIZE);
        lcd.getTextBounds(ammo_str, AMMO_TEXT_START_X, AMMO_TEXT_START_Y, &x1, &y1, &w, &h);
        lcd.fillRect(x1, y1, w, h, ST77XX_BLACK);
    }

    void lcdDriver::clear_clip_count() {
        int16_t  x1, y1;
        uint16_t w, h;
        lcd.setTextSize(AMMO_TEXT_SIZE);
        lcd.getTextBounds(clips_str, CLIP_TEXT_START_X, CLIP_TEXT_START_Y, &x1, &y1, &w, &h);
        lcd.fillRect(x1, y1, w, h, ST77XX_BLACK);
    }

    void lcdDriver::clear_reload_str() {
        int16_t x1, y1;
        uint16_t w, h;
        lcd.setTextSize(RELOAD_TEXT_SIZE);
        lcd.getTextBounds(reload_str, RELOAD_CENTER_X + RELOAD_TEXT_X_OFFSET,
                          RELOAD_CENTER_Y + RELOAD_TEXT_Y_OFFSET, &x1, &y1, &w, &h);
        lcd.fillRect(x1, y1, w, h, ST77XX_BLACK);
    }

    void lcdDriver::clear() {
        this->clear_screen();
    }

    void lcdDriver::toggle_backlight() {
        digitalToggleFast(BACKLIGHT_PIN);
    }

    // ------------------------------------------------------MENU METHODS----------------------------------------------------//

    void lcdDriver::clear_canvas(){
        canvas.fillScreen(ST77XX_BLACK);
    }

    void lcdDriver::draw_canvas() { // Draws the canvas without blanking the screen
       lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 240);
    }

    void lcdDriver::draw_menu(){ // Draws the currently loaded menu
        clear_canvas();
        canvas.fillScreen(current_menu->background_color);
        canvas.setCursor(0, 0);
        canvas.setTextSize(3);
        canvas.setTextColor(current_menu->text_color);
        canvas.print(current_menu->name);
        // Calculate the bounds of the menu name and start the menu items from there
        int16_t x1, y1;
        uint16_t w, h;
        uint16_t w2, h2;
        canvas.getTextBounds(current_menu->name, 5, 0, &x1, &y1, &w, &h);
        int16_t start_x = x1 + w + 5;
        int16_t start_y = y1 + h + 5;
        canvas.setCursor(0, start_y);
        canvas.setTextSize(2);
        // Disable text wraping
        canvas.setTextWrap(false);
        // Calculate how many items need to be skipped so that the selected item is on the screen
        int16_t skip_items = 0;
        for (int i = 0; i < current_menu->num_items; i++) {
            // Calculate the bounds of all the items before the selected item
            canvas.getTextBounds(current_menu->items[i].name, start_x, start_y + i * h, &x1, &y1, &w, &h);
            if (current_menu->items[i].sub_menu != nullptr) h = h * 2;
            // If the selected item is not on the screen skip items until it is
            if (y1 + h >= LCD_HEIGHT - h) {
                if (i == current_menu->selected_item) {
                    skip_items = i;
                }
            }
        }
        char* num_str = new char[3];
        for (int i = skip_items; i < current_menu->num_items; i++) {
            canvas.setTextColor(current_menu->text_color);
            if (i == current_menu->selected_item) {
                if (current_menu->items[i].func != nullptr || current_menu->items[i].func_param != nullptr) {
                    sprintf(num_str, "%02d>", i + 1);
                    canvas.setTextColor(ST77XX_GREEN);
                } else sprintf(num_str, "%02d-", i + 1);
            } else sprintf(num_str, "%02d:", i + 1);
            canvas.print(num_str);
            canvas.print(current_menu->items[i].name);
            canvas.getTextBounds(current_menu->items[i].name, 0, start_y, &x1, &y1, &w, &h);
            if (current_menu->items[i].sub_menu != nullptr) {
                canvas.setCursor(0, start_y + h + 3);
                if (current_menu->items[i].sub_menu->is_active) {
                    canvas.setTextColor(ST77XX_YELLOW);
                    canvas.print("=>");
                } else canvas.print("->");
                auto* name = current_menu->items[i].sub_menu->option_names
                        [current_menu->items[i].sub_menu->selected_option]->c_str();
                canvas.print(name);
                canvas.getTextBounds(num_str, 0, start_y + i * h + 3, &x1, &y1, &w2, &h2);
                start_y += h + 3;
            }
            start_y += h + 3;
            canvas.setCursor(0, start_y);
        }
        canvas.setTextWrap(true);
        delete[] num_str;
        draw_canvas();
    }

    void lcdDriver::load_free_display_menu(menu_holder* menu) {
        free_menu(current_menu);
        current_menu = new menu_holder(*menu);
        draw_menu();
    }

    void lcdDriver::load_and_display_menu(menu_holder *menu) {
//        free_menu(current_menu);
        // Make a copy of the new menu otherwise if the old menu is ceased to exist the
        this->current_menu = menu;
        this->draw_menu();
    }

    menu_holder *lcdDriver::make_menu(const char *name, uint16_t text_color, uint16_t background_color) {
        auto *menu = new menu_holder;
        char* name_ptr = new char[strlen(name) + 1];
        strcpy(name_ptr, name);
        menu->name = name_ptr;
        menu->text_color = text_color;
        menu->background_color = background_color;
        menu->num_items = 0;
        menu->selected_item = 0;
        menu->items = static_cast<menu_item *>(malloc(sizeof(menu_item) * MAX_MENU_ITEMS));
        return menu;
    }

    menu_holder *lcdDriver::make_menu(const char *name) {
        return lcdDriver::make_menu(name, ST77XX_WHITE, ST77XX_BLACK);
    }

    void lcdDriver::add_menu_item(menu_holder *menu, const char *name, void (*func)()) {
        if (menu->num_items < MAX_MENU_ITEMS) {
            char* name_ptr = new char[strlen(name) + 1];
            strcpy(name_ptr, name);
            menu->items[menu->num_items].name = name_ptr;
            menu->items[menu->num_items].func = func;
            menu->items[menu->num_items].func_param = nullptr;
            menu->items[menu->num_items].sub_menu = nullptr;
            menu->num_items++;
        }
    }

    void lcdDriver::add_menu_item(menu_holder *menu, const char *name, void (*func)(int), int arg){
        if (menu->num_items < MAX_MENU_ITEMS) {
            char* name_ptr = new char[strlen(name) + 1];
            strcpy(name_ptr, name);
            menu->items[menu->num_items].name = name_ptr;
            menu->items[menu->num_items].func = nullptr;
            menu->items[menu->num_items].func_param = func;
            menu->items[menu->num_items].func_arg = arg;
            menu->items[menu->num_items].sub_menu = nullptr;
            menu->num_items++;
        }
    }

    void lcdDriver::add_menu_item(menu_holder *menu, const char *name) {
        if (menu->num_items < MAX_MENU_ITEMS) {
            char* name_ptr = new char[strlen(name) + 1];
            strcpy(name_ptr, name);
            menu->items[menu->num_items].name = name_ptr;
            menu->items[menu->num_items].func = nullptr;
            menu->items[menu->num_items].func_param = nullptr;
            menu->items[menu->num_items].sub_menu = nullptr;
            menu->num_items++;
        }
    }

    void lcdDriver::menu_increment() {
        if (current_menu->items[current_menu->selected_item].sub_menu != nullptr &&
            current_menu->items[current_menu->selected_item].sub_menu->is_active){
            current_menu->items[current_menu->selected_item].sub_menu->selected_option++;
            if (current_menu->items[current_menu->selected_item].sub_menu->selected_option >=
                current_menu->items[current_menu->selected_item].sub_menu->num_options) {
                current_menu->items[current_menu->selected_item].sub_menu->selected_option = 0;
            }
        } else {
            current_menu->selected_item++;
            if (current_menu->selected_item >= current_menu->num_items) {
                current_menu->selected_item = 0;
            }
        }
        clear_screen();
        draw_menu();
    }

    void lcdDriver::menu_decrement() {
        // Check if the current item has a sub menu, if so, go cycle through the sub menu items instead
        if (current_menu->items[current_menu->selected_item].sub_menu != nullptr &&
            current_menu->items[current_menu->selected_item].sub_menu->is_active){
            if (current_menu->items[current_menu->selected_item].sub_menu->selected_option > 0) {
                current_menu->items[current_menu->selected_item].sub_menu->selected_option--;
            } else {
                current_menu->items[current_menu->selected_item].sub_menu->selected_option =
                current_menu->items[current_menu->selected_item].sub_menu->num_options - 1; // Go to the last option
//                if (current_menu->items[current_menu->selected_item].sub_menu->selected_option <= 0) {
//
                }
        } else { // If the current item doesn't have a sub menu, decrement the current item
            current_menu->selected_item--;
            if (current_menu->selected_item < 0) {
                current_menu->selected_item = current_menu->num_items - 1;
            }
        }
        clear_screen();
        draw_menu();
    }

    void lcdDriver::menu_select(bool select) {
        if (select) {
            if (current_menu->items[current_menu->selected_item].sub_menu != nullptr) {
                if (current_menu->items[current_menu->selected_item].sub_menu->is_active) {
                    if (current_menu->items[current_menu->selected_item].func_param != nullptr)
                        current_menu->items[current_menu->selected_item].func_param(
                                current_menu->items[current_menu->selected_item].sub_menu->selected_option);
                    current_menu->items[current_menu->selected_item].sub_menu->is_active = false;
                } else current_menu->items[current_menu->selected_item].sub_menu->is_active = true;
                clear_screen();
                draw_menu();
            } else if (current_menu->items[current_menu->selected_item].func != nullptr) {
                current_menu->items[current_menu->selected_item].func();
            } else if (current_menu->items[current_menu->selected_item].func_param != nullptr) {
                current_menu->items[current_menu->selected_item].func_param
                (current_menu->items[current_menu->selected_item].func_arg);
            }
        }
    }

    menu_option_item *lcdDriver::add_submenu(menu_holder *menu, const char* name, void (*func)(int)) {

        char* name_ptr = new char[strlen(name) + 1];
        strcpy(name_ptr, name);
        menu->items[menu->num_items].name = name_ptr;
        menu->items[menu->num_items].func = nullptr;
        menu->items[menu->num_items].func_param = func;
        auto* item = new menu_option_item;
        item->option_names = new String*[MAX_OPTION_MENU_OPTIONS];
        for (int i = 0; i < MAX_OPTION_MENU_OPTIONS; i++) {
            item->option_names[i] = new String("null");
        }
        item->num_options = 0;
        item->selected_option = 0;
        menu->items[menu->num_items].sub_menu = item;
        menu->num_items++;
        return item;
    }

    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step) {
        for (unsigned int i = 0; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(i);
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, uint32_t start, unsigned int range, unsigned int step) {
        for (uint32_t i = start; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(i);
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step,
                                       const char* unit) {
        for (unsigned int i = 0; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(String(i) + " " + String(unit));
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, uint32_t start, uint32_t range,
                                       unsigned int step, const char* unit) {
        for (uint32_t i = start; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(String(i) + " " + String(unit));
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    void lcdDriver::add_submenu_item(menu_option_item *sub_menu, const char *name_new) {
        delete sub_menu->option_names[sub_menu->num_options];
        auto *name = new String(name_new);
        sub_menu->option_names[sub_menu->num_options] = name;
        sub_menu->num_options++;
    }

    void lcdDriver::submenu_set_selected(menu_option_item *menu, unsigned int selected) {
        if (selected < menu->num_options) {
            menu->selected_option = selected;
        } else {
            menu->selected_option = MAX_OPTION_MENU_OPTIONS - 1;
        }
    }

    void lcdDriver::free_menu(menu_holder *menu) {
        if (menu != nullptr) {
            for (unsigned int i = 0; i < menu->num_items; i++) {
                delete[] menu->items[i].name;
                if (menu->items[i].sub_menu != nullptr) {
                    for (unsigned int j = 0; j < menu->items[i].sub_menu->num_options; j++) {
                        delete[] menu->items[i].sub_menu->option_names[j];
                    }
                    delete[] menu->items[i].sub_menu->option_names;
                    delete menu->items[i].sub_menu;
                }
            }
            delete[] menu->items;
            delete menu;
        }
    }

} // display