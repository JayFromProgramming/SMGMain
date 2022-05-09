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

#include <Adafruit-ST7735-Library-master/Adafruit_ST7789.h>    // Core graphics library
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
#define RELOAD_RADIUS        100
#define RELOAD_INNER_RADIUS  50
#define RELOAD_TEXT_X_OFFSET -35
#define RELOAD_TEXT_Y_OFFSET -20
#define RELOAD_TEXT_SIZE     4


#define DEATH_TEXT_START_X   24
#define DEATH_TEXT_START_Y   10
#define DEATH_TEXT_SIZE      3

#define MAX_MENU_ITEMS       32
#define MAX_OPTION_MENU_OPTIONS  255
#define MAX_OPTION_LENGTH   32

Adafruit_ST7789 lcd = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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
            if (already_reloading){ // Update reloading animation
                float remaining_reload = ((float) this->game_state->reload_time - (float) millis()) / 1000;
                float remaining_reload_percent = (float) remaining_reload /
                        (float) this->game_state->currentConfig->reload_time;
                float reload_angle = ((1 - remaining_reload_percent) * 360.0f) - 90.0f;
                // Sweep the circle with a line based on the remaining reload time to make a full circle
                // The line starts from the outer edge of the inner circle and ends at the outer edge of the outer circle
                for (float angle = last_angle; angle < reload_angle; angle += 0.1f) {
                    int x = RELOAD_CENTER_X + RELOAD_RADIUS * cos(angle * M_PI / 180);
                    int y = RELOAD_CENTER_Y + RELOAD_RADIUS * sin(angle * M_PI / 180);
                    int x2 = RELOAD_CENTER_X + RELOAD_INNER_RADIUS * cos(angle * M_PI / 180);
                    int y2 = RELOAD_CENTER_Y + RELOAD_INNER_RADIUS * sin(angle * M_PI / 180);
                    lcd.drawLine(x, y, x2, y2, ST77XX_GREEN);
                }
                last_angle = reload_angle;

                // Print the remaining reload time in seconds
                if (remaining_reload < 10) {
                    sprintf(reload_str, "%.1f", remaining_reload);
                } else if (remaining_reload < 99) {
                    sprintf(reload_str, "99");
                }else sprintf(reload_str, "%d", (unsigned short)  remaining_reload);

                last_reload_time = (unsigned short) remaining_reload;

                if (strcmp(reload_str, old_reload_str) != 0) {
                    lcd.setCursor(RELOAD_CENTER_X + RELOAD_TEXT_X_OFFSET, RELOAD_CENTER_Y + RELOAD_TEXT_Y_OFFSET);
                    lcd.setTextSize(RELOAD_TEXT_SIZE);
                    lcd.setTextColor(ST77XX_GREEN);
                    this->clear_reload_str();
                    lcd.print(reload_str);
                    strcpy(old_reload_str, reload_str);
                }

            } else { // Start reloading animation
                already_reloading = true;
                this->clear_clip_count();
                this->clear_ammo_count();
                lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_RADIUS, ST77XX_YELLOW);
                lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_INNER_RADIUS, ST77XX_BLACK);
            }
        } else {
            if (!this->game_state->started){
                tagger_init_screen();
            }
            if (already_reloading) { // Stop reloading animation
                already_reloading = false;
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
        lcd.setTextSize(5);
        lcd.setTextColor(ST77XX_GREEN);
        lcd.print("Hold Trigger to Start");
        lcd.setCursor(0, 12);
        lcd.setTextSize(2);
        // Print some info about current config

    }

    void lcdDriver::draw_death_screen() {
        clear_screen();
        lcd.fillScreen(ST77XX_RED);
        lcd.setCursor(DEATH_TEXT_START_X, DEATH_TEXT_START_Y);
        lcd.setTextSize(DEATH_TEXT_SIZE);
        lcd.setTextColor(ST77XX_WHITE);
        unsigned long time_alive = millis() - this->score->respawn_time;
        unsigned long time_alive_seconds = time_alive / 1000;
        unsigned long time_alive_minutes = time_alive_seconds / 60;
        sprintf(time_alive_str, "%lu:%02lu", time_alive_minutes, time_alive_seconds % 60);
        sprintf(death_str, "GAME OVER!\n"
                           "Shots fired:\n%d\n"
                           "Time alive:\n%13s\n"
                           "Killed by:\n%d", score->rounds_fired, time_alive_str, score->killed_by);
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

    void lcdDriver::draw_menu(){ // Draws the currently loaded menu
        clear_screen();
        lcd.fillScreen(current_menu->background_color);
        lcd.setCursor(0, 0);
        lcd.setTextSize(3);
        lcd.setTextColor(current_menu->text_color);
        lcd.print(current_menu->name);
        // Calculate the bounds of the menu name and start the menu items from there
        int16_t x1, y1;
        uint16_t w, h;
        uint16_t w2, h2;
        lcd.getTextBounds(current_menu->name, 5, 0, &x1, &y1, &w, &h);
        int16_t start_x = x1 + w + 5;
        int16_t start_y = y1 + h + 5;
        lcd.setCursor(0, start_y);
        lcd.setTextSize(2);
        // Disable text wraping
        lcd.setTextWrap(false);
        // Calculate how many items need to be skipped so that the selected item is on the screen
        int16_t skip_items = 0;
        for (int i = 0; i < current_menu->num_items; i++) {
            // Calculate the bounds of all the items before the selected item
            lcd.getTextBounds(current_menu->items[i].name, start_x, start_y + i * h, &x1, &y1, &w, &h);
            // If the selected item is not on the screen skip items until it is
            if (y1 + h >= LCD_HEIGHT - h) {
                if (i == current_menu->selected_item) {
                    skip_items = i;
                }
            }
        }
        char* num_str = new char[3];
        for (int i = skip_items; i < current_menu->num_items; i++) {
            if (i == current_menu->selected_item) {
                if (current_menu->items[i].func != nullptr || current_menu->items[i].func_param != nullptr) {
                    sprintf(num_str, "%02d>", i + 1);
                } else sprintf(num_str, "%02d-", i + 1);
            } else sprintf(num_str, "%02d:", i + 1);
            lcd.print(num_str);
            lcd.print(current_menu->items[i].name);
            lcd.getTextBounds(current_menu->items[i].name, 0, start_y, &x1, &y1, &w, &h);
            if (current_menu->items[i].sub_menu != nullptr) {
                lcd.setCursor(0, start_y + h + 3);
                lcd.print("->");
                auto* name = current_menu->items[i].sub_menu->option_names
                        [current_menu->items[i].sub_menu->selected_option]->c_str();
                lcd.print(name);
                lcd.getTextBounds(num_str, 0, start_y + i * h + 3, &x1, &y1, &w2, &h2);
                start_y += h + 3;
            }
            start_y += h + 3;
            lcd.setCursor(0, start_y);
        }
        lcd.setTextWrap(true);
        delete[] num_str;
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
        current_menu->selected_item++;
        if (current_menu->selected_item >= current_menu->num_items) {
            current_menu->selected_item = 0;
        }
        clear_screen();
        draw_menu();
    }

    void lcdDriver::menu_decrement() {
        current_menu->selected_item--;
        if (current_menu->selected_item < 0) {
            current_menu->selected_item = current_menu->num_items - 1;
        }
        clear_screen();
        draw_menu();
    }

    void lcdDriver::menu_select(bool select) {
        if (select) {
            if (current_menu->items[current_menu->selected_item].sub_menu != nullptr) {

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
        item->num_options = 0;
        item->option_names = new String*[MAX_OPTION_MENU_OPTIONS];
        item->selected_option = 0;
        menu->items[menu->num_items].sub_menu = item;
        menu->num_items++;
        return item;
    }

    void lcdDriver::add_option_menu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step) {
        for (unsigned int i = 0; i < range; i += step) {
            auto *name = new String(i);
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    void lcdDriver::add_option_menu_item(menu_option_item *sub_menu, const char *name) {
        auto* name_ptr = new String(name);
        sub_menu->option_names[sub_menu->num_options] = name_ptr;
        sub_menu->num_options++;
    }

    void lcdDriver::option_menu_set_selected(menu_option_item *menu, unsigned int selected) {
        menu->selected_option = selected;
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