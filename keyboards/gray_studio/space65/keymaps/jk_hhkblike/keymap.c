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
  VVV_DFT = SAFE_RANGE,
  VVV_RST
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_BL] = LAYOUT( \
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL, KC_BSLS,  KC_GRV,  KC_CALC,  \
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
        _______, XXXXXXX, VVV_DFT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_SLCK, KC_BRK,  KC_UP,   XXXXXXX,          _______, _______,  \
        _______, KC_VOLD, KC_VOLU, KC_MUTE, KC_EJCT, _______, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT,                   KC_PENT, _______,  \
        _______, _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PPLS, KC_PMNS, KC_END,  KC_PGDN, KC_DOWN,          _______, _______, _______,  \
        VVV_RST,   _______, _______,                   _______, _______, _______,        _______, VVV_RST,                   _______, _______, _______   \
    ),
};

typedef struct {
    uint8_t current_value;
    uint8_t target_value;
    uint8_t max_value;
    uint8_t min_value;
    float smooth_param;
} vvv_smooth;

static uint16_t vvv_smooth_last_update;
vvv_smooth smooth_vals[3];
vvv_smooth * p_smooth_val = &smooth_vals[0];
vvv_smooth * p_smooth_sat = &smooth_vals[1];
vvv_smooth * p_smooth_hue = &smooth_vals[2];

void vvv_smooth_init(vvv_smooth * val, uint8_t start_val, uint8_t start_target, uint8_t min, uint8_t max, float param) {
  val->current_value = start_val;
  val->target_value = start_target;
  val->min_value = min;
  val->max_value = max;
  val->smooth_param = param;
}

void vvv_smooth_update(vvv_smooth * val) {
  val->current_value = val->current_value * (1-val->smooth_param) + val->target_value * val->smooth_param;
}

void vvv_smooth_add_target(vvv_smooth * val, int add) {
  int new_val = val->target_value + add;
  if (new_val > val->max_value) val->target_value = val->max_value;
  else if (new_val < val->min_value) val->target_value = val->min_value;
  else val->target_value = (uint8_t) new_val;
}

void vvv_smooth_set_target(vvv_smooth * val, uint8_t target) {
  if (target > val->max_value) val->target_value = val->max_value;
  else if (target < val->min_value) val->target_value = val->min_value;
  else val->target_value = target;
}

void set_base_layer_led(uint8_t layer) {
  switch (biton32(layer)) {
    case _WL:
      vvv_smooth_set_target(p_smooth_hue, 16);
      break;
    case _BL:
    default:
      vvv_smooth_set_target(p_smooth_hue, 28);
      break;
  }
}

void matrix_init_user(void) {
  vvv_smooth_last_update = timer_read();
  vvv_smooth_init(p_smooth_val, 0, 230, 0, 230, 0.035);
  vvv_smooth_init(p_smooth_sat, 0, 230, 1, 230, 0.1);
  vvv_smooth_init(p_smooth_hue, 0,  23, 1, 255, 0.15);
}

void vvv_update(void) {
  vvv_smooth_add_target(p_smooth_sat, 4);
  vvv_smooth_update(p_smooth_val);
  vvv_smooth_update(p_smooth_sat);
  vvv_smooth_update(p_smooth_hue);
  rgblight_sethsv_master(p_smooth_hue->current_value, 230, p_smooth_val->current_value);
  rgblight_sethsv_slave(28,   p_smooth_sat->current_value, p_smooth_val->current_value);
}

void matrix_scan_user(void) {
  if (timer_elapsed(vvv_smooth_last_update) > 20) {
    vvv_smooth_last_update = timer_read();
    vvv_update();
  }
}


// Set default color etc. at startup
void keyboard_post_init_user(void) {
  rgblight_enable();
  rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
  rgblight_sethsv(255, 1, 1);
  set_base_layer_led(eeconfig_read_default_layer());
}

// When layer state switches.
uint32_t layer_state_set_user(uint32_t state) {
  switch (biton32(state)) {
    case _FL:
      vvv_smooth_set_target(p_smooth_hue, 100);
      break;
    default:
      set_base_layer_led(default_layer_state);
      break;
  }
  return state;
}

// TODO: just messing around here. Maybe visualize all modifiers?
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case VVV_RST:
      if (record->event.pressed) break;
      rgblight_sethsv_master(240, 235, 235);
      rgblight_sethsv_slave(1, 0, 0);
      reset_keyboard();
      return false;
    case VVV_DFT:
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
      vvv_smooth_add_target(p_smooth_sat, -55);
      break;
  }
  return true;
}

// For capslock etc.
void led_set_user(uint8_t usb_led) {
}
