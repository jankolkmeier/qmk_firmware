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

enum layers_idx {
    _BL, // Base Layer
    _WL, // Windows Modifiers Layer (Switched around LGUI, LALT, LCTL)
    _FL  // Functions
};

enum custom_keycodes {
  RGB_MYT = SAFE_RANGE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BL] = LAYOUT( \
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL, KC_BSLS,  KC_GRV,  KC_HOME,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,         KC_BSPC,  KC_PGUP,  \
        KC_LGUI, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,                   KC_ENT,  KC_PGDN,  \
        KC_LSFT, KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,         KC_RSFT,   KC_UP,  KC_END,   \
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
        RESET,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, KC_MRWD,  \
        _______, _______, DF(_WL), _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          KC_DEL,  KC_VOLU,  \
        _______, _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,                   _______, KC_VOLD,  \
        _______, _______, _______, _______, _______, _______, _______, _______, DF(_BL), _______, _______, _______,          _______, _______, KC_MFFD,  \
        _______, _______, _______,                   RGB_MYT, RGB_MYT, RGB_MYT,          _______, _______,                   _______, _______, _______   \
    ),
};

static uint16_t hsv_pulse_last_update;
static uint8_t hsv_pulse_power;
static uint8_t hsv_pulse_current;
static uint8_t hsv_base_value;
static uint8_t hsv_min_value;

void matrix_init_user(void) {
  hsv_base_value = 235;
  hsv_min_value = 25;
  hsv_pulse_current = hsv_base_value;
  hsv_pulse_power = 0;
  hsv_pulse_last_update = timer_read();
}

void update_hsv_pulse(void) {
  // Pulse power decays over time
  int power_delta = hsv_base_value - hsv_pulse_power;
  hsv_pulse_power = hsv_pulse_power + power_delta/abs(power_delta) * 3;

  // Color fades towards target
  int pulse_delta = hsv_pulse_power - hsv_pulse_current;
  hsv_pulse_current = hsv_pulse_current + pulse_delta * 0.3;

  rgblight_sethsv_slave(28, hsv_pulse_current, 230);
}

void add_hsv_pulse(uint8_t power) {
  int sum = hsv_pulse_power - power;
  if (sum < hsv_min_value) hsv_pulse_power = hsv_min_value;
  else hsv_pulse_power = (uint8_t) sum;
}

void matrix_scan_user(void) {
  if (timer_elapsed(hsv_pulse_last_update) > 20) {
    hsv_pulse_last_update = timer_read();
    update_hsv_pulse();
  }
}


// Set default color etc. at startup
void keyboard_post_init_user(void) {
  rgblight_enable();
  rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
  rgblight_sethsv( 28, 230, 230);
}

// When layer state switches.
uint32_t layer_state_set_user(uint32_t state) {
  switch (biton32(state)) {
    case _FL:
      rgblight_sethsv_master(102, 230, 230);
      break;
    default:
      switch (biton32(default_layer_state)) {
        case _WL:
          rgblight_sethsv_master(16, 230, 230);
          break;
        case _BL:
        default:
          rgblight_sethsv_master(28, 230, 230);
          break;
      }
      break;
  }
  return state;
}

// TODO: just messing around here. Maybe visualize all modifiers?
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case RGB_MYT:
      if (record->event.pressed) {
        add_hsv_pulse(255);
      }
      break;
    default:
      if (record->event.pressed) {
      } else {
        add_hsv_pulse(55);
      }
      break;
  }
  return true;
}

// For capslock etc.
void led_set_user(uint8_t usb_led) {
}
