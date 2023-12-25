// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include <stdio.h>
#include "i2c_master.h"
#include "pimoroni_trackball.h"
#define _BASE 0
#define _RA 1
#define _BL 2
#define ___ KC_TRNS

enum my_keycodes {
  SLEEP = QK_KB_0,
  TB_CYCLE
};

static uint16_t idle_timer = 0;
static uint16_t seconds_counter = 0;
static bool rgb_led_idle_turnoff = false;
static bool arm_sleep = false;
static uint16_t arm_sleep_timer = 0;

extern int trackball_active_mode;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BASE] = LAYOUT_ansi( \
     KC_ESC,            KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6,    KC_F7,  KC_F8,    KC_F9,  KC_F10,  KC_F11,  KC_F12,KC_PSCR,  KC_SCRL, KC_PAUSE, \
     KC_GRV, KC_1,       KC_2,  KC_3,  KC_4,  KC_5,  KC_6,  KC_7,     KC_8,   KC_9,     KC_0, KC_MINS,  KC_EQL, KC_BSPC, KC_INS,  KC_HOME,  KC_PGUP,  KC_NUM, KC_PSLS, KC_PAST, KC_PMNS, \
     KC_TAB, KC_Q,       KC_W,  KC_E,  KC_R,  KC_T,  KC_Y,  KC_U,     KC_I,   KC_O,     KC_P, KC_LBRC, KC_RBRC, KC_BSLS, KC_DEL,   KC_END,  KC_PGDN,   KC_P7,   KC_P8,   KC_P9, KC_PPLS, \
    KC_CAPS, KC_A,       KC_S,  KC_D,  KC_F,  KC_G,  KC_H,  KC_J,     KC_K,   KC_L,  KC_SCLN, KC_QUOT,  KC_ENT,                                        KC_P4,   KC_P5,   KC_P6,          \
    KC_LSFT, KC_Z,       KC_X,  KC_C,  KC_V,  KC_B,  KC_N,  KC_M,  KC_COMM, KC_DOT, KC_SLASH,          KC_RSFT,                     KC_UP,             KC_P1,   KC_P2,   KC_P3, KC_PENT, \
    KC_LCTL, KC_LGUI, KC_LALT,              KC_SPC,                        KC_RALT,  MO(_RA),  KC_APP, KC_RCTL,          KC_LEFT, KC_DOWN, KC_RIGHT,   KC_P0,          KC_PDOT \
),
[_RA] = LAYOUT_ansi( \
      SLEEP,              ___,   ___,KC_F13,KC_F14,KC_F15,KC_F16,   KC_F17, KC_F18,  KC_F19,  KC_F20,  KC_F21,  KC_F22, KC_F23,   KC_F24,      ___, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,     ___,  NK_ON,      ___,      ___,     ___,     ___,     ___,     ___, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,     ___, NK_OFF,      ___,      ___,     ___,     ___,     ___,     ___, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,                                          ___,     ___,     ___,          \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,              ___,                   KC_VOLU,               ___,     ___,     ___,     ___, \
        ___,AG_TOGG,      ___,                 ___,                           ___,      ___, MO(_BL),     ___,            ___,   KC_VOLD,      ___,     ___,              ___ \
),
[_BL] = LAYOUT_ansi( \
        ___,              ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,     ___,    ___,   QK_RBT,  QK_BOOT, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,     ___,    ___,      ___,      ___,     ___,     ___,     ___, RGB_VAD, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,     ___,    ___,      ___,      ___,     ___, RGB_HUI,     ___, RGB_VAI, \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,     ___,     ___,                                      RGB_SAD, RGB_TOG, RGB_SAI,          \
        ___,  ___,        ___,   ___,   ___,   ___,   ___,   ___,      ___,    ___,     ___,              ___,                   RGB_VAD,               ___, RGB_HUD,     ___,     ___, \
        ___,  ___,        ___,             RGB_MOD,                            ___,     ___,     ___,TB_CYCLE,             ___,  RGB_VAI,      ___, RGB_MOD,              ___ \
)
};

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    static char rgbStatusLine1[16] = {0};
    static char rgbStatusLine2[16] = {0};
    led_t led_state = host_keyboard_led_state();
    // Host Keyboard Layer Status
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("BASE"), get_highest_layer(layer_state) == _BASE);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("RA"), get_highest_layer(layer_state) == _RA);
    oled_write_P(PSTR("  "), false);
    oled_write_P(PSTR("BL\n"), get_highest_layer(layer_state) == _BL);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("NUM"), led_state.num_lock);
    oled_write_P(PSTR("  "), false);
    oled_write_P(PSTR("CAP"), led_state.caps_lock);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("SCR"), led_state.scroll_lock);
    oled_write_P(PSTR("\n "), false);
    oled_write_P(PSTR("RGB"), rgblight_is_enabled());
    snprintf(rgbStatusLine1, sizeof(rgbStatusLine1), " M:%2d h:%3d \n", rgblight_get_mode(), rgblight_get_hue());
    snprintf(rgbStatusLine2, sizeof(rgbStatusLine2), " s:%3d v:%3d", rgblight_get_sat(), rgblight_get_val());
    oled_write_P(PSTR(rgbStatusLine1), false);
    oled_write_P(PSTR(rgbStatusLine2), false);
    oled_write_P(PSTR(" "), false);
    switch (trackball_active_mode) {
        case TRACKBALL_MODE_MOUSE:
            oled_write_P(PSTR("TB: MOU"), false);
            break;
        case TRACKBALL_MODE_CURSOR:
            oled_write_P(PSTR("TB: CUR"), false);
            break;
        case TRACKBALL_MODE_VOLUME:
            oled_write_P(PSTR("TB: VOL"), false);
            break;
        case TRACKBALL_MODE_RGB_CONFIG:
            oled_write_P(PSTR("TB: RGB"), false);
            break;
    }
    return false;
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#if defined(RGBLIGHT_TIMEOUT) && defined(RGBLIGHT_ENABLE)
    if (record->event.pressed) {
        idle_timer = timer_read();
        seconds_counter = 0;
        if(rgb_led_idle_turnoff) {
            rgb_led_idle_turnoff = false;
            rgblight_enable_noeeprom();
            trackball_color_auto();
        }
    }
#endif
  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %u, row: %u, pressed: %i, time: %u, interrupt: %i, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif
    switch (keycode) {
        case SLEEP:
            if (record->event.pressed) {
#if defined(RGBLIGHT_ENABLE)
                rgb_led_idle_turnoff = true;
                rgblight_disable_noeeprom();
#endif
                arm_sleep = true;
                arm_sleep_timer = timer_read();
                trackball_set_rgbw(0x00, 0x00, 0x00, 0x00);
            }
            return false; // Skip all further processing of this key
        case TB_CYCLE:
            if (record->event.pressed) {
                if (trackball_active_mode == (TRACKBALL_MODE_END - 1)) {
                    trackball_active_mode = TRACKBALL_MODE_MOUSE;
                } else {
                    ++trackball_active_mode;
                }
                trackball_color_auto();
            }
            return false;
        default:
            return true; // Process all other keycodes normally
    }
    return true;
}

void matrix_scan_user(void) {
#if defined(RGBLIGHT_TIMEOUT) && defined(RGBLIGHT_ENABLE)
    if (idle_timer == 0 || rgb_led_idle_turnoff){
        idle_timer = timer_read();
        seconds_counter = 0;
    } else {
        if(timer_elapsed(idle_timer) > 1000) {
            ++seconds_counter;
            idle_timer = timer_read();
        }
        if(rgblight_is_enabled() && seconds_counter > RGBLIGHT_TIMEOUT) {
            rgb_led_idle_turnoff = true;
            rgblight_disable_noeeprom();
            trackball_set_rgbw(0x00, 0x00, 0x00, 0x00);
        }
    }
#endif
    if (arm_sleep == true && timer_elapsed(arm_sleep_timer) > 1000) {
        arm_sleep = false;
#if defined(OLED_ENABLE)
        oled_off();
#endif
    }
}


void suspend_power_down_user(void) {
    trackball_set_rgbw(0x00, 0x00, 0x00, 0x00);
}

void suspend_wakeup_init_user(void) {
    trackball_color_auto();
}