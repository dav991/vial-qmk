/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "i2c_master.h"
#include "pimoroni_trackball.h"

static uint8_t scrolling      = 0;
static int16_t x_offset       = 0;
static int16_t y_offset       = 0;
static int16_t h_offset       = 0;
static int16_t v_offset       = 0;
static float   precisionSpeed = 1;
int trackball_active_mode = TRACKBALL_MODE_VOLUME;
static int16_t vertical_accumulator = 0;
static int16_t horizontal_accumulator = 0;
static int16_t accumulator_divider = 20;

#ifndef I2C_TIMEOUT
#    define I2C_TIMEOUT 100
#endif
#ifndef MOUSE_DEBOUNCE
#    define MOUSE_DEBOUNCE 5
#endif

void trackball_set_rgbw(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    uint8_t data[] = {0x00, red, green, blue, white};
    i2c_transmit(TRACKBALL_WRITE, data, sizeof(data), I2C_TIMEOUT);
}

int16_t mouse_offset(uint8_t positive, uint8_t negative, int16_t scale) {
    int16_t offset    = (int16_t)positive - (int16_t)negative;
    int16_t magnitude = (int16_t)(scale * offset * offset * precisionSpeed);
    return offset < 0 ? -magnitude : magnitude;
}

void update_member(int8_t* member, int16_t* offset) {
    if (*offset > 127) {
        *member = 127;
        *offset -= 127;
    } else if (*offset < -127) {
        *member = -127;
        *offset += 127;
    } else {
        *member = *offset;
        *offset = 0;
    }
}

void trackball_color_auto() {
    switch(trackball_active_mode){
        case TRACKBALL_MODE_MOUSE:
            trackball_set_rgbw(0x00, 0x00, 0x00, 0x88);
            break;
        case TRACKBALL_MODE_CURSOR:
            trackball_set_rgbw(0x88, 0x00, 0x88, 0x33);
            break;
        case TRACKBALL_MODE_VOLUME:
            trackball_set_rgbw(0x00, 0x88, 0x00, 0x33);
            break;
        case TRACKBALL_MODE_RGB_CONFIG:
            trackball_set_rgbw(0x88, 0x88, 0x00, 0x00);
            break;
        default:
            trackball_set_rgbw(0xFF, 0x00, 0x00, 0x00);
    }
}

__attribute__((weak)) void trackball_check_click(bool pressed, report_mouse_t* mouse) {
    if (pressed) {
        mouse->buttons |= MOUSE_BTN1;
    } else {
        mouse->buttons &= ~MOUSE_BTN1;
    }
}

void trackball_register_button(bool pressed, enum mouse_buttons button) {
    report_mouse_t currentReport = pointing_device_get_report();
    if (pressed) {
        currentReport.buttons |= button;
    } else {
        currentReport.buttons &= ~button;
    }
    pointing_device_set_report(currentReport);
}

float trackball_get_precision(void) { return precisionSpeed; }
void  trackball_set_precision(float precision) { precisionSpeed = precision; }
bool  trackball_is_scrolling(void) { return scrolling; }
void  trackball_set_scrolling(bool scroll) { scrolling = scroll; }

bool has_report_changed (report_mouse_t first, report_mouse_t second) {
    return !(
        (!first.buttons && first.buttons == second.buttons) &&
        (!first.x && first.x == second.x) &&
        (!first.y && first.y == second.y) &&
        (!first.h && first.h == second.h) &&
        (!first.v && first.v == second.v) );
}


__attribute__((weak)) void pointing_device_init(void) { trackball_color_auto(); }

bool pointing_device_task(void) {
    static bool     debounce;
    static uint16_t debounce_timer;
    static bool     was_tapped = false;
    uint8_t         state[5] = {};
    if (i2c_readReg(TRACKBALL_WRITE, 0x04, state, 5, I2C_TIMEOUT) == I2C_STATUS_SUCCESS) {
        if (!state[4] && !debounce) {
                x_offset += mouse_offset(state[2], state[3], 5);
                y_offset += mouse_offset(state[1], state[0], 5);
        } else {
            if (state[4]) {
                debounce       = true;
                debounce_timer = timer_read();
            }
        }
    }
#ifdef CONSOLE_ENABLE
    if (state[0] || state[1] || state[2] || state[3] || state[4])
        uprintf("trackball: %i %i %i %i %i %i %i\n", state[0], state[1], state[2], state[3], state[4], vertical_accumulator, horizontal_accumulator);
#endif
    if (timer_elapsed(debounce_timer) > MOUSE_DEBOUNCE) debounce = false;
    if (trackball_active_mode == TRACKBALL_MODE_MOUSE)
    {
        report_mouse_t mouse = pointing_device_get_report();

        trackball_check_click(state[4] & (1 << 7), &mouse);

    #ifndef PIMORONI_TRACKBALL_ROTATE
        update_member(&mouse.x, &x_offset);
        update_member(&mouse.y, &y_offset);
        update_member(&mouse.h, &h_offset);
        update_member(&mouse.v, &v_offset);
    #else
        update_member(&mouse.x, &y_offset);
        update_member(&mouse.y, &x_offset);
        update_member(&mouse.h, &v_offset);
        update_member(&mouse.v, &h_offset);
    #endif
        pointing_device_set_report(mouse);
        if (has_report_changed(mouse, pointing_device_get_report())) {
            return pointing_device_send();
        }
    }
    else if (trackball_active_mode == TRACKBALL_MODE_CURSOR)
    {
        vertical_accumulator   += mouse_offset(state[0], state[1], 1);
        horizontal_accumulator += mouse_offset(state[2], state[3], 1);
        if (state[4] & (1 << 7))
        {
            if (!was_tapped) {
                register_code(KC_MUTE);
                was_tapped = true;
            }
        }else{
            if(was_tapped)
            {
                unregister_code(KC_MUTE);
                was_tapped = false;
            }
        }
        if (vertical_accumulator > 0 && vertical_accumulator/accumulator_divider != 0)
        {
            tap_code(KC_UP);
        }
        else if (vertical_accumulator < 0 && vertical_accumulator/accumulator_divider != 0 )
        {
            tap_code(KC_DOWN);
        }
        if (horizontal_accumulator > 0 && horizontal_accumulator/accumulator_divider != 0)
        {
            tap_code(KC_RIGHT);
        }
        else if (horizontal_accumulator < 0 && horizontal_accumulator/accumulator_divider != 0)
        {
            tap_code(KC_LEFT);
        }

        vertical_accumulator = vertical_accumulator%accumulator_divider;
        horizontal_accumulator = horizontal_accumulator%accumulator_divider;
    }
    else if (trackball_active_mode == TRACKBALL_MODE_VOLUME)
    {
        vertical_accumulator   += mouse_offset(state[0], state[1], 1);
        horizontal_accumulator += mouse_offset(state[2], state[3], 1);
        if (state[4] & (1 << 7))
        {
            if (!was_tapped) {
                register_code(KC_MUTE);
                was_tapped = true;
            }
        }else{
            if(was_tapped)
            {
                unregister_code(KC_MUTE);
                was_tapped = false;
            }
        }
        if (vertical_accumulator > 0 && vertical_accumulator/accumulator_divider != 0)
        {
#ifdef CONSOLE_ENABLE
            uprintf("trackball: tap volume up\n");
#endif
            //tap_code16(LCTL(LSFT(KC_PGUP)));
            tap_code16(KC_VOLU);
        }
        else if (vertical_accumulator < 0 && vertical_accumulator/accumulator_divider != 0 )
        {
#ifdef CONSOLE_ENABLE
            uprintf("trackball: tap volume down\n");
#endif
            //tap_code16(LCTL(LSFT(KC_PGDN)));
            tap_code16(KC_VOLD);
        }
        vertical_accumulator = vertical_accumulator%accumulator_divider;
        horizontal_accumulator = horizontal_accumulator%accumulator_divider;
    }
    else if (trackball_active_mode == TRACKBALL_MODE_RGB_CONFIG)
    {
#if defined(RGBLIGHT_ENABLE)
        vertical_accumulator   += mouse_offset(state[0], state[1], 1);
        horizontal_accumulator += mouse_offset(state[2], state[3], 1);
        if (state[4] & (1 << 7) )
        {
            if (!was_tapped) {
                rgblight_toggle_noeeprom();
                was_tapped = true;
            }
        }else{
            if (was_tapped) {
                was_tapped = false;
            }
        }
        if (vertical_accumulator > 0 && vertical_accumulator/accumulator_divider != 0)
        {
            rgblight_increase_hue();
        }
        else if (vertical_accumulator < 0 && vertical_accumulator/accumulator_divider != 0 )
        {
            rgblight_decrease_hue();
        }
        if (horizontal_accumulator > 0 && horizontal_accumulator/accumulator_divider != 0)
        {
            rgblight_increase_val();
        }
        else if (horizontal_accumulator < 0 && horizontal_accumulator/accumulator_divider != 0)
        {
            rgblight_decrease_val();
        }
        vertical_accumulator = vertical_accumulator%accumulator_divider;
        horizontal_accumulator = horizontal_accumulator%accumulator_divider;
#endif
    }
    return false;
}
