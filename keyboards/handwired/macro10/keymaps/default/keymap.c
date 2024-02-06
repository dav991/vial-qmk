// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT(
        KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,
        KC_A,    KC_S,    KC_D,    KC_F,    MO(1)
    ),
    [1] = LAYOUT(
        _______, _______, _______, _______, MO(2),
        _______, _______, _______, _______, _______
    ),
    [2] = LAYOUT(
        _______, _______, _______, MO(3), _______,
        _______, _______, _______, _______, _______
    ),
    [3] = LAYOUT(
        _______, _______, _______, _______, _______,
        _______, _______, _______, QK_BOOT, _______
    )
};

#ifdef RGBLIGHT_LAYERS
const rgblight_segment_t PROGMEM my_layer1_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 5, HSV_BLUE}
);
const rgblight_segment_t PROGMEM my_layer2_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 5, HSV_PURPLE}
);
const rgblight_segment_t PROGMEM my_layer3_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 5, HSV_GREEN}
);
const rgblight_segment_t PROGMEM my_layer4_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 5, HSV_RED}
);
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(

    my_layer1_layer,   
    my_layer2_layer,   
    my_layer3_layer,
    my_layer4_layer
);

void keyboard_post_init_user(void) {
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
}

bool led_update_user(led_t led_state) {
    return true;
}

layer_state_t default_layer_state_set_user(layer_state_t state) {
    
    return state;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    rgblight_set_layer_state(0, layer_state_cmp(state, 1));
    rgblight_set_layer_state(1, layer_state_cmp(state, 2));
    rgblight_set_layer_state(2, layer_state_cmp(state, 3));
    rgblight_set_layer_state(3, layer_state_cmp(state, 4));
    return state;
}
#endif