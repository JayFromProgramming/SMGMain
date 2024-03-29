//
// Created by Jay on 4/20/2022.
//

//                lcd.drawLine(RELOAD_CENTER_X + cos(reload_angle) * RELOAD_INNER_RADIUS,
//                             RELOAD_CENTER_Y + sin(reload_angle) * RELOAD_INNER_RADIUS,
//                             RELOAD_CENTER_X + cos(reload_angle) * RELOAD_RADIUS,
//                             RELOAD_CENTER_Y + sin(reload_angle) * RELOAD_RADIUS,
//                             ST77XX_GREEN);

#include "lcdDriver.h"
#include <pinout.h>
#include "../../.pio/libdeps/teensylc/Adafruit GFX Library/Adafruit_GFX.h"

#include "Adafruit-ST7735-Library-master/Adafruit_ST7789.h"
//#include <Adafruit_GFX.h>
#include <SPI.h>
#include <gfxfont.h>

//#define DEBUG_MODE

#define LCD_HEIGHT    240

#define HEALTH_BAR_START_X  7
#define HEALTH_BAR_START_Y  10
#define HEALTH_BAR_WIDTH    233
#define HEALTH_BAR_HEIGHT   10

#define SHIELD_BAR_START_X  7
#define SHIELD_BAR_START_Y  20
#define SHIELD_BAR_WIDTH    233
#define SHIELD_BAR_HEIGHT   10

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

Adafruit_ST7789 lcd = Adafruit_ST7789(DISPLAY_CHIP_SELECT, DISPLAY_DC, DISPLAY_RST);

// Define a canvas
GFXcanvas16 canvas(240, 240);

bool backlight = true;
bool backlight_forced = false;

namespace display {

    /**
     * @brief Initialize the LCD display
     */
    void lcdDriver::displayInit() {
        lcd.init(240, 240, SPI_MODE0);
        pinMode(DISPLAY_BACKLIGHT, OUTPUT);
        digitalWriteFast(DISPLAY_BACKLIGHT, HIGH);
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

    /**
     * @brief Pass game data pointers to the display driver
     * @param data - Pointer to a tagger_state_struct struct
     * @param score_t - Pointer to score_data_struct struct
     * @see tagger_state_struct, score_data_struct
     */
    void lcdDriver::pass_data_ptr(tagger_state_struct *data, score_data_struct *score_t) {
        clips_str = static_cast<char *>(malloc(sizeof(char) * 10));
        ammo_str  = static_cast<char *>(malloc(sizeof(char) * 10));
        reload_str = static_cast<char *>(malloc(sizeof(char) * 10));
        old_progress_str = static_cast<char *>(malloc(sizeof(char) * 10));
        health_str = static_cast<char *>(malloc(sizeof(char) * 10));
        death_str = static_cast<char *>(malloc(sizeof(char) * 64));
        time_alive_str = static_cast<char *>(malloc(sizeof(char) * 14));
        this->game_state = data;
        this->score = score_t;
    }

    /**
     * @brief Updates the display during the game loop
     * @details This function is called every cycle of the game loop, but only updates the parts of the display that
     * have changed since the last update. This reduces the amount of SPI traffic and makes the system more readable.
     */
    void lcdDriver::update_hud() {
        // Check if any game state values have changed to determine if we need to update the screen

#ifdef DEBUG_MODE
        if (Serial) {
            Serial.println("Updating HUD...");
            // Dump certain values to the serial port
            Serial.printf("Displaying alert: %d\n"
                          "Displaying progress circle: %d\n"
                          "\tProgress: %d\n"
                          "Game state: %d\n\tHealth: %d\n\tShield: %d\n\tClips: %d\n\tAmmo: %d\n"
                          "\tReload: %d\n",
                          displaying_alert,
                          display_progress_circle,
                          progress_circle_total_time - progress_circle_timer,
                          game_state, game_state->health, game_state->shield_health,
                          game_state->clip_count, game_state->ammo_count);
            // Dump the serial buffer before continuing so we get complete data before it crashes
            Serial.flush();
        }
#endif
        if (displaying_alert) return; // Don't display the hud if an alert is being displayed

        if (display_progress_circle){
            progress_circle();
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
//        this->draw_system_elements();

    }

    void lcdDriver::start_progress_circle(uint32_t total_milliseconds) {
        start_progress_circle(total_milliseconds, nullptr);
    }

    /**
     * @brief Draws a timed progress circle, can be used to represent reloading and respawn time, etc.
     * @details This function draws 2 circles one inside the other, the inner circle is black to allow the time text
     * to be readable. The outer circle is slowly overwritten by lines of green to allow for a slowly moving line to
     * fill the circle.
     * @param total_milliseconds - The total time in milliseconds that the circle will take to fill
     * @param header - The header text to display above the circle
     */
    void lcdDriver::start_progress_circle(uint32_t total_milliseconds, String* header){
        display_progress_circle = true;
        this->progress_circle_total_time = total_milliseconds;
        this->progress_circle_timer = 0;
        this->clear_clip_count();
        this->clear_ammo_count();
        this->reset_angle = true;
        if (header != nullptr) {
            clear_screen();
            lcd.setCursor(RELOAD_CENTER_X - RELOAD_RADIUS, RELOAD_CENTER_Y - RELOAD_RADIUS);
            lcd.setTextSize(RELOAD_TEXT_SIZE);
            lcd.setTextColor(ST77XX_WHITE);
            lcd.print(*header->c_str());
        }
        lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_RADIUS, ST77XX_YELLOW);
        lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_INNER_RADIUS, ST77XX_BLACK);
    }


    void lcdDriver::cancel_progress_circle() {
        display_progress_circle = false;

        lcdDriver::clear_screen();
        this->last_health = -1;
        this->last_clip_count = -1;
        this->last_ammo_count = -1;
        clear_screen();
    }

    void lcdDriver::progress_circle() {
        float remaining_time = ((float) progress_circle_total_time - (float) progress_circle_timer) / 1000.0f;
        float remaining_reload_percent =  (float) progress_circle_timer.operator unsigned long() /
                (float) progress_circle_total_time;
//        if (remaining_reload_percent > 1.0f) remaining_reload_percent = 1.0f;
        float remaining_angle = -((1 - remaining_reload_percent) * 360.0f) - 90.0f;
        if (reset_angle) last_angle = remaining_angle;
        reset_angle = false;
//        Serial.printf("Rem angle: %f\n", remaining_angle);
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

        last_progress_time = (unsigned short) remaining_time;

        if (strcmp(reload_str, old_progress_str) != 0) {
            lcd.setCursor(RELOAD_CENTER_X + RELOAD_TEXT_X_OFFSET, RELOAD_CENTER_Y + RELOAD_TEXT_Y_OFFSET);
            lcd.setTextSize(RELOAD_TEXT_SIZE);
            lcd.setTextColor(ST77XX_GREEN);
            this->clear_reload_str();
            lcd.print(reload_str);
            strcpy(old_progress_str, reload_str);
        }

        if (progress_circle_timer > progress_circle_total_time) {
            cancel_progress_circle(); // Stop the progress circle if the time is up
        }

    }

    float_t lcdDriver::calc_health_percentage() {
        return (float)game_state->health / (float) game_state->max_health;
    }

    float_t lcdDriver::calc_shield_percentage() {
        return (float)game_state->shield_health / (float) game_state->max_shield_health;
    }

    /**
     * Clears the entire screen
     */
    void lcdDriver::clear_screen() {
        lcd.fillScreen(ST77XX_BLACK);
    }

    /**
     * Displayed when the game has not started yet
     */
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
    void lcdDriver::draw_system_elements() {
//        lcd.setCursor(0, 45);
//        lcd.setTextSize(1);
//        int16_t x, y;
//        uint16_t w, h;
//        String str = String(this->game_state->last_shot);
//        lcd.getTextBounds(str.c_str(), 0, 45, &x, &y, &w, &h);
//        lcd.fillRect(x, y, w, h, ST77XX_BLACK);
//        lcd.print(this->game_state->last_shot);
    }

    /**
     * @brief Draws the death screen containing the game over message and the score
     */
    void lcdDriver::draw_death_screen() {
        clear_screen();
        lcd.fillScreen(ST77XX_RED);
        lcd.setCursor(DEATH_TEXT_START_X, DEATH_TEXT_START_Y);
        lcd.setTextSize(DEATH_TEXT_SIZE);
        lcd.setTextColor(ST77XX_WHITE);
        Serial.println("Rendering death screen\nCalculating time alive");
        uint32_t time_alive = this->score->last_alive_time;
        uint32_t time_alive_seconds = time_alive / 1000;
        uint32_t time_alive_minutes = time_alive_seconds / 60;
        sprintf(time_alive_str, "%lu:%02lu", time_alive_minutes, time_alive_seconds % 60);
        String killer;
        Serial.println("Calculating killer");
        if (this->score->killer_name != nullptr && this->score->assist_name != nullptr) {
            killer = String(*this->score->killer_name + " + " + *this->score->assist_name);
        } else if (this->score->killer_name != nullptr) {
            killer = String(*this->score->killer_name);
        } else
            killer = String("Unknown");

        Serial.println("Drawing death screen");
        sprintf(death_str, "GAME OVER!\n"
                           "Shots fired:\n%d\n"
                           "Time active:\n%13s\n"
                           "Eliminated by\n%s", score->rounds_fired_game, time_alive_str, killer.c_str());
        lcd.print(death_str);
    }

    /**
     * @brief Draws the health bar at the top of the screen
     */
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
        } else {
            draw_horizontal_percent_bar(HEALTH_BAR_START_X, HEALTH_BAR_START_Y, HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT,
                                        calc_health_percentage(), ST77XX_GREEN, ST77XX_RED);
            draw_horizontal_percent_bar(SHIELD_BAR_START_X, SHIELD_BAR_START_Y, SHIELD_BAR_WIDTH, SHIELD_BAR_HEIGHT,
                                        calc_shield_percentage(), ST77XX_BLUE, ST77XX_BLACK);
        }
    }

    /**
     * @brief Draws the ammo count
     */
    void lcdDriver::draw_ammo_count() {
        sprintf(ammo_str, "%02d", game_state->ammo_count < 99 ? game_state->ammo_count : 99);
        lcd.setCursor(AMMO_TEXT_START_X, AMMO_TEXT_START_Y);
        lcd.setTextSize(AMMO_TEXT_SIZE);
        if (game_state->ammo_count > 5) { // If ammo count is above 5 ammo count is green
            lcd.setTextColor(ST77XX_GREEN);
        } else if (game_state->ammo_count > 0) { // If ammo count is below 5 ammo count is yellow
            lcd.setTextColor(ST77XX_YELLOW);
        } else { // If player has no ammo, the ammo count is red
            lcd.setTextColor(ST77XX_RED);
        }
        lcd.print(ammo_str);
    }

    /**
     * @brief Draws the clip count
     */
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

    /**
     * @brief Clears the area occupied by the ammo count
     */
    void lcdDriver::clear_ammo_count() {
        int16_t  x1, y1;
        uint16_t w, h;
        lcd.setTextSize(AMMO_TEXT_SIZE);
        lcd.getTextBounds(ammo_str, AMMO_TEXT_START_X, AMMO_TEXT_START_Y, &x1, &y1, &w, &h);
        lcd.fillRect(x1, y1, w, h, ST77XX_BLACK);
    }

    /**
     * @brief Clears the area occupied by the clip count
     */
    void lcdDriver::clear_clip_count() {
        int16_t  x1, y1;
        uint16_t w, h;
        lcd.setTextSize(AMMO_TEXT_SIZE);
        lcd.getTextBounds(clips_str, CLIP_TEXT_START_X, CLIP_TEXT_START_Y, &x1, &y1, &w, &h);
        lcd.fillRect(x1, y1, w, h, ST77XX_BLACK);
    }

    /**
     * @brief Draws the progress circles remaining time string
     */
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
        this->last_health = -1;
        this->last_ammo_count = -1;
        this->last_clip_count = -1;
    }

    void lcdDriver::force_backlight(bool value, bool force) {
        backlight_forced = force;
        digitalWriteFast(DISPLAY_BACKLIGHT, value ? HIGH : LOW);
        if (!force) backlight = value;
    }

    void lcdDriver::force_backlight(bool force) {
        if (backlight_forced){
            digitalWriteFast(DISPLAY_BACKLIGHT, backlight ? HIGH : LOW);
        }
        backlight_forced = force;
    }

    /**
     * @brief Toggles the LCD backlight
     */
    void lcdDriver::toggle_backlight() {
        if (backlight_forced) {
            return;
        }
        backlight = !backlight;
        digitalWriteFast(DISPLAY_BACKLIGHT, backlight ? HIGH : LOW);
    }

    void lcdDriver::display_alert(String* title, String* info){
        this->displaying_alert = true;
        this->clear();
        lcd.setCursor(0, 0);
        lcd.setTextSize(6);
        lcd.setTextColor(ST77XX_RED);
        lcd.print(title->c_str());

    }

    void lcdDriver::clear_alert(){
        this->displaying_alert = false;
        this->clear();
    }

    void lcdDriver::draw_horizontal_percent_bar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                                float_t percent, uint16_t mainColor, uint16_t secondaryColor){
        // To minimize draw time we draw the bar in two parts
        // First we calculate each part's width, height is constant

        if (percent > 1.0f) percent = 1.0f; // Clamp to 1.0f
        if (percent < 0.0f) percent = 0.0f; // Clamp to 0.0f
        uint16_t x1 = x; // First part's x
        uint16_t x2 = x + (uint16_t) (w * percent); // Calculate the second part's x position
        auto w1 = (uint16_t) (w * percent); // Calculate the first part's width
        uint16_t w2 = x2 - x; // Calculate the second part's width
        // First we draw the main part of the bar
        lcd.fillRect(x1, y, w1, h, mainColor);
        // Then we draw the secondary part of the bar
        lcd.fillRect(x2, y, w2, h, secondaryColor);
    }

    // ------------------------------------------------------MENU METHODS----------------------------------------------------//

    /**
     * Clears the display buffer
     */
    void lcdDriver::clear_canvas(){
        canvas.fillScreen(ST77XX_BLACK);
    }

    /**
     * @brief Draws the display buffer to the LCD
     */
    void lcdDriver::draw_canvas() { // Draws the canvas without blanking the screen
       lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 240);
    }

    /**
     * @brief Draws the current menu to the display buffer
     */
    void lcdDriver::draw_menu(){ // Draws the currently loaded menu
        clear_canvas();
        if (current_menu != nullptr) {
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
            char *num_str = new char[3];
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
                    auto *name = current_menu->items[i].sub_menu->option_names
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
        }
        draw_canvas();
    }

    /**
     * @details Loads a new menu and draws it on the screen.
     * @param menu - A menu_holder pointer to the menu to be loaded.
     * @note This menu makes a copy of the menu passed to it the original menu is not freed.
     */
    void lcdDriver::load_free_display_menu(menu_holder* menu) {
        free_menu(current_menu);
        current_menu = new menu_holder(*menu);
        draw_menu();
    }

    /**
     * @detals Frees the memory allocated to a menu_holder and all of its associated pointers
     * @param menu - A pointer to the menu_holder to free
     * @bug It just doesn't work :(
     */
    void lcdDriver::free_menu(menu_holder *menu) {
        if (menu != nullptr) {
//            Serial.printf("Freeing menu %s\n", menu->name);
            for (unsigned int i = 0; i < menu->num_items; i++) {
//                Serial.println("\tFreeing item");
                delete[] menu->items[i].name;
                if (menu->items[i].sub_menu != nullptr) {
                    for (unsigned int j = 0; j < menu->items[i].sub_menu->num_options; j++) {
//                        Serial.println("\t\tFreeing option name");
                        delete menu->items[i].sub_menu->option_names[j];
                    }
//                    Serial.println("\t\tFreeing option name array");
                    delete[] menu->items[i].sub_menu->option_names;
//                    Serial.println("\t\tFreeing sub menu pointer");
                    delete menu->items[i].sub_menu;
                }
            }
//            Serial.println("\tFreeing item array");
            delete[] menu->items;
            delete menu;
        }
//        Serial.println("Menu freed");
        menu = nullptr;
    }

    /**
     * @details Loads a new menu and draws it on the screen.
     * @param menu - A menu_holder pointer to the menu to be loaded.
     * @note This menu makes a copy of the menu passed to it the original menu is not freed.
     * @warning The old menu is not freed and will be lost if not freed before calling this method.
     * @deprecated This method is prone to memory leaks and should not be used.
     */
    void lcdDriver::load_and_display_menu(menu_holder *menu) {
        this->current_menu = menu;
        this->draw_menu();
    }

    /**
     * @details Creates a new menu holder and returns a pointer to it.
     * @param name - A string pointer to the name of the menu.
     * @param text_color  - The color of the text in the menu.
     * @param background_color  - The color of the background of the menu.
     * @return A pointer to the newly created menu holder.
     */
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

    /**
     * @details Creates a new menu holder and returns a pointer to it.
     * @param name - A string pointer to the name of the menu.
     * @return A pointer to the newly created menu holder.
     */
    menu_holder *lcdDriver::make_menu(const char *name) {
        return lcdDriver::make_menu(name, ST77XX_WHITE, ST77XX_BLACK);
    }

    /**
     * @details Creates a new menu item and returns a pointer to it.
     * @param menu - A menu_holder pointer to the menu to which the item belongs.
     * @param name - A string pointer to the name of the item.
     * @param func - A function pointer to the function to be called when the item is selected.
     * @return A pointer to the newly created menu item.
     */
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

    /**
     * @details Creates a new menu item and returns a pointer to it.
     * @param menu - A menu_holder pointer to the menu to which the item belongs.
     * @param name - A string pointer to the name of the item.
     * @param func - A function pointer with an int argument to be called when the item is selected.
     * @param arg -  The argument to be passed to the function when it is called.
     * @return A pointer to the newly created menu item.
     */
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

    /**
     * @details Creates a new menu item and returns a pointer to it.
     * @param menu - A menu_holder pointer to the menu to which the item belongs.
     * @param name - A string pointer to the name of the item.
     * @return A pointer to the newly created menu item.
     */
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

    /**
     * @details Increments what item is currently selected
     */
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
        if (current_menu->selected_item >= current_menu->num_items) {
            current_menu->selected_item = 0;
        }
        clear_screen();
        draw_menu();
    }

    /**
     * @details Decrements what item is currently selected
     */
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

    /**
     * @details Executes the function of the currently selected item, or it will select the sub menu if it exists
     * @param select - A boolean values to indicate falling or rising edge
     */
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

    /**
     * @details Adds a new submenu to the passed menu
     * @param menu - The menu to add the sub menu to
     * @param name - The name of the sub menu
     * @param func - The function (int) to execute when the sub menu is exited
     * @return A pointer to the new sub menu, nullptr if the sub menu could not be added
     */
    menu_option_item *lcdDriver::add_submenu(menu_holder *menu, const char* name, void (*func)(int)) {
        if (menu->num_items >= MAX_MENU_ITEMS) {
            return nullptr;
        }
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

    /**
     * @details Adds options to the passed sub menu in a range
     * @param sub_menu* - The sub menu to add the options to
     * @param range - The end of the range of options to add
     * @param step  - The step size of the range
     */
    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step) {
        for (unsigned int i = 0; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(i);
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    /**
     * @details Adds options to the passed sub menu in a range
     * @param sub_menu* - A pointer to the sub menu to add the options to
     * @param start - The start of the range of options to add
     * @param range - The end of the range of options to add
     * @param step  - The step size of the range
     */
    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, uint32_t start, unsigned int range, unsigned int step) {
        for (uint32_t i = start; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(i);
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    /**
     * @details Adds options to the passed sub menu in a range
     * @param sub_menu - A pointer to the sub menu to add the options to
     * @param range - The end of the range of options to add
     * @param step - The step size of the range
     * @param unit - A unit string to append to the end of the option value
     */
    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, unsigned int range, unsigned int step,
                                       const char* unit) {
        for (unsigned int i = 0; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(String(i) + " " + String(unit));
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    /**
     * @details Adds options to the passed sub menu in a range
     * @param sub_menu - A pointer to the sub menu to add the options to
     * @param start - The start of the range of options to add
     * @param range - The end of the range of options to add
     * @param step - The step size of the range
     * @param unit - A unit string to append to the end of the option value
     */
    void lcdDriver::add_submenu_values(menu_option_item *sub_menu, uint32_t start, uint32_t range,
                                       unsigned int step, const char* unit) {
        for (uint32_t i = start; i < range; i += step) {
            delete sub_menu->option_names[sub_menu->num_options];
            auto *name = new String(String(i) + " " + String(unit));
            sub_menu->option_names[sub_menu->num_options] = name;
            sub_menu->num_options++;
        }
    }

    /**
     * @details Adds a single option to the passed sub menu
     * @param sub_menu - A pointer to the sub menu to add the option to
     * @param name_new - The name of the option to add
     */
    void lcdDriver::add_submenu_item(menu_option_item *sub_menu, const char *name_new) {
        delete sub_menu->option_names[sub_menu->num_options];
        auto *name = new String(name_new);
        sub_menu->option_names[sub_menu->num_options] = name;
        sub_menu->num_options++;
    }

    /**
     * Sets the default selected option for the passed sub menu
     * @param menu  - A pointer to the sub menu to set the default option for
     * @param selected - The index of the option to set as the default
     */
    void lcdDriver::submenu_set_selected(menu_option_item *menu, int16_t selected) {
        if (selected < menu->num_options) {
            menu->selected_option = selected;
        } else {
            menu->selected_option = menu->num_options - 1;
//            menu->selected_option = MAX_OPTION_MENU_OPTIONS - 1;
        }
    }



} // display