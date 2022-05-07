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

#define HEALTH_BAR_START_X  7
#define HEALTH_BAR_START_Y  10
#define HEALTH_BAR_WIDTH    233
#define HEALTH_BAR_HEIGHT   10

#define CLIP_TEXT_START_X   50
#define CLIP_TEXT_START_Y   30
#define CLIP_TEXT_SIZE      4

#define AMMO_TEXT_START_X   64
#define AMMO_TEXT_START_Y   76
#define AMMO_TEXT_SIZE      14

#define RELOAD_CENTER_X      160
#define RELOAD_CENTER_Y      160
#define RELOAD_RADIUS        75
#define RELOAD_INNER_RADIUS  35
#define RELOAD_TEXT_X_OFFSET -10
#define RELOAD_TEXT_Y_OFFSET -10
#define RELOAD_TEXT_SIZE     5

Adafruit_ST7789 lcd = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

namespace display {

    void lcdDriver::displayInit() {
        lcd.init(240, 240, SPI_MODE3);
        lcd.setRotation(2);
        lcd.fillScreen(ST77XX_RED);
        lcd.fillScreen(ST77XX_GREEN);
        lcd.fillScreen(ST77XX_BLUE);
        lcd.fillScreen(ST77XX_WHITE);
        lcd.fillScreen(ST77XX_BLACK);
        lcd.setTextSize(7);
        lcd.print("Hello World!");
    }

    void lcdDriver::pass_data_ptr(tagger_state *data) {
        clips_str = static_cast<char *>(malloc(sizeof(char) * 10));
        ammo_str  = static_cast<char *>(malloc(sizeof(char) * 10));
        reload_str = static_cast<char *>(malloc(sizeof(char) * 10));
        health_str = static_cast<char *>(malloc(sizeof(char) * 10));
        this->game_state = data;
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
                float reload_angle = (1 - remaining_reload_percent) * 360.0f;
                // Sweep the circle with a line based on the remaining reload time to make a full circle
                // The line starts from the outer edge of the inner circle and ends at the outer edge of the outer circle
                for (float angle = last_angle; angle < reload_angle; angle += 0.5) {
                    int x = RELOAD_CENTER_X + RELOAD_RADIUS * cos(angle * M_PI / 180);
                    int y = RELOAD_CENTER_Y + RELOAD_RADIUS * sin(angle * M_PI / 180);
                    int x2 = RELOAD_CENTER_X + RELOAD_INNER_RADIUS * cos(angle * M_PI / 180);
                    int y2 = RELOAD_CENTER_Y + RELOAD_INNER_RADIUS * sin(angle * M_PI / 180);
                    lcd.drawLine(x, y, x2, y2, ST77XX_GREEN);
                }
                last_angle = reload_angle;
                if ((unsigned short) remaining_reload != last_reload_time) {
                    this->clear_reload_str();
                    lcd.setCursor(RELOAD_CENTER_X + RELOAD_TEXT_X_OFFSET, RELOAD_CENTER_Y + RELOAD_TEXT_Y_OFFSET);
                    lcd.setTextSize(RELOAD_TEXT_SIZE);
                    lcd.setTextColor(ST77XX_GREEN);
                    // Print the remaining reload time in seconds
                    sprintf(reload_str, "%2d", (unsigned short) remaining_reload);
                    lcd.print(reload_str);
                    last_reload_time = (unsigned short) remaining_reload;
                }

            } else { // Start reloading animation
                already_reloading = true;
                this->clear_clip_count();
                this->clear_ammo_count();
                lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_RADIUS, ST77XX_YELLOW);
                lcd.fillCircle(RELOAD_CENTER_X, RELOAD_CENTER_Y, RELOAD_INNER_RADIUS, ST77XX_BLACK);
            }
        } else {
            if (already_reloading) { // Stop reloading animation
                already_reloading = false;
                last_angle = 0;
                lcdDriver::clear_screen();
                this->last_health = -1;
                this->last_clip_count = -1;
                this->last_ammo_count = -1;
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

    void lcdDriver::draw_health_bar() {
        if (this->game_state->health <= 0) {
            lcd.fillRect(7, 10, 233, 10, ST77XX_RED);
        } else if (this->game_state->health == this->game_state->max_health) {
            lcd.fillRect(7, 10, 233, 10, ST77XX_GREEN);
        } else {
            lcd.fillRect((int)(calc_health_percentage() * 226) + 7, 10, 233, 7, ST77XX_RED);
            lcd.fillRect(7, 10, (int)(calc_health_percentage() * 226) + 7, 7, ST77XX_GREEN);
        }
    }

    void lcdDriver::draw_ammo_count() {
        sprintf(ammo_str, "%2d", game_state->ammo_count);
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

//    void lcdDriver::override_text(String* text) {
//        lcd.fillScreen(ST77XX_BLACK);
//        lcd.setCursor(0, 0);
//        lcd.setTextSize(11);
//        lcd.setTextColor(ST77XX_WHITE);
//        lcd.print(text);
//    }


} // display