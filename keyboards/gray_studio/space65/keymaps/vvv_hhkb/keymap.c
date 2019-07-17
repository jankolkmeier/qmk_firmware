/* Copyright 2019 MechMerlin
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
#include QMK_KEYBOARD_H

#define _______ KC_TRNS

// SMOOTHING ENGINE START
#define VVV_SMOOTH_NVALS 3
#define VVV_SMOOTH_INTERVAL 20
#include "vvv.h"

enum vvv_smooth_vals {
  total_brightness = 0,
  type_pulse,
  layer_hue
};

// Color of the slave light
const uint8_t slave_hue = 23;
// Color of the layers: there has to be one number for each layer!
const uint8_t layer_hues[] = {
    23,  // _BL
    16,  // _WL
    100  // _FL
};

enum layers_idx {
    _BL, // Base Layer
    _WL, // Windows Modifiers Layer (Switched around LGUI, LALT, LCTL)
    _FL  // Functions
};

enum custom_keycodes {
  VVV_DFT = SAFE_RANGE,
  VVV_RST
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BL] = LAYOUT( \
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL, KC_BSLS,  KC_GRV,  KC_ESC,   \
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,         KC_BSPC,  KC_PGUP,  \
        KC_LGUI, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,                   KC_ENT,  KC_PGDN,  \
        KC_LSFT, KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,         KC_RSFT,   KC_UP,  MO(_FL),  \
        MO(_FL), KC_LALT, KC_LCTL,                   KC_SPC,  KC_SPC,  KC_SPC,           KC_RALT, MO(_FL),                  KC_LEFT, KC_DOWN,  KC_RGHT   \
    ),
[_WL] = LAYOUT( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,  \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,  \
        KC_LCTL, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                   _______, _______,  \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______,  \
        _______, KC_LGUI, KC_LALT,                   _______, _______, _______,          _______, _______,                   _______, _______, _______   \
    ),
[_FL] = LAYOUT( \
        KC_POWER,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10, KC_F11, KC_F12,  KC_INS,  KC_DEL,  _______,  \
        _______, XXXXXXX, VVV_DFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_SLCK, KC_BRK,  KC_UP,   XXXXXXX,          _______, KC_HOME,  \
        _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_EJCT, XXXXXXX, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT,                   KC_PENT, KC_END,   \
        _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PPLS, KC_PMNS, KC_END,  KC_PGDN, KC_DOWN,          _______, _______, VVV_RST,  \
        VVV_RST,   _______, _______,                   _______, _______, _______,        _______, VVV_RST,                   _______, _______, _______   \
    ),
};


// Set default color etc. at startup
void keyboard_post_init_user(void) {
  // enable LEDs, but turn them off. The smoothing engine will fade them in.
  rgblight_enable();
  rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
  rgblight_sethsv(1, 1, 1);
  vvv_smooth_set_target(layer_hue, layer_hues[biton32(eeconfig_read_default_layer())]);
}

void matrix_init_user(void) {
  // Setup smoothing engine
  vvv_smooth_init(total_brightness, 0, 230, 0, 230, 0.035);
  vvv_smooth_init(type_pulse, 235, 235, 80, 255, 0.1);
  vvv_smooth_init(layer_hue, 0,  slave_hue, 1, 255, 0.15);
}

void matrix_scan_user(void) {
  // the smoothing engine will update at the defined frequency... if it updates,
  // the function returns true and we adjust the behavior further:
  if (vvv_smooth_update()) {
    vvv_smooth_add_target(type_pulse, 4);
    rgblight_sethsv_master(vvv_get_val(layer_hue), 235, vvv_get_val(total_brightness));
    rgblight_sethsv_slave(slave_hue, vvv_get_val(type_pulse), vvv_get_val(total_brightness));
  }
}

// When layer state switches.
uint32_t layer_state_set_user(uint32_t state) {
  uint8_t layer = biton32(state);
  uint8_t default_layer = biton32(default_layer_state);
  vvv_smooth_set_target(layer_hue, layer_hues[layer]);
  if (layer <= default_layer) {
    vvv_smooth_set_target(layer_hue, layer_hues[default_layer]);
  }
  return state;
}

// TODO: just messing around here. Maybe visualize all modifiers?
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case VVV_RST: // RESET THE KEYBOARD
      if (record->event.pressed) break;
      rgblight_sethsv_master(245, 235, 235);
      rgblight_sethsv_slave(1, 0, 0);
      reset_keyboard();
      return false;
    case VVV_DFT: // TOGGLE DEFAULT LAYER (TODO: more elegant solution here)
      if (record->event.pressed) break;
      switch (biton32(default_layer_state)) {
        case _WL:
          set_single_persistent_default_layer(_BL);
          return false;
        case _BL:
        default:
          set_single_persistent_default_layer(_WL);
          return false;
      }
      break;
    default:
      if (record->event.pressed) break;
      vvv_smooth_add_target(type_pulse, -55);
      break;
  }
  return true;
}

// For capslock etc.
void led_set_user(uint8_t usb_led) {
}
