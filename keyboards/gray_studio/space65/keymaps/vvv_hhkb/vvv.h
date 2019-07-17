/* author: jk
 * = WIP =
 * simple header for setting up multiple values that smooth-interpolate to a target over time.
 *
 * TODOs:
 *  - make more generic/allow values of type other than uint8_t
 *  - use a smoothing parameter that is dependent on the time/update frequency, to be more predictable
 *  - ...
 */
#pragma once

#ifndef VVV_SMOOTH_NVALS
  #define VVV_SMOOTH_NVALS 1
#endif

#ifndef VVV_SMOOTH_INTERVAL
  #define VVV_SMOOTH_INTERVAL 20
#endif

typedef struct {
    uint8_t current_value;
    uint8_t target_value;
    uint8_t max_value;
    uint8_t min_value;
    float smooth_param;
} vvv_smooth;

static uint16_t vvv_smooth_last_update;

uint8_t vvv_get_val(uint8_t val);
void vvv_smooth_init(uint8_t val, uint8_t start_val, uint8_t start_target, uint8_t min, uint8_t max, float param);
void vvv_smooth_add_target(uint8_t val, int add);
void vvv_smooth_set_target(uint8_t val, uint8_t target);
bool vvv_smooth_update(void);

vvv_smooth vvv_smooth_vals[VVV_SMOOTH_NVALS];

// Initialize a smooth value
void vvv_smooth_init(uint8_t val, uint8_t start_val, uint8_t start_target, uint8_t min, uint8_t max, float param) {
  if (val >= VVV_SMOOTH_NVALS) return;
  vvv_smooth_last_update = timer_read();
  vvv_smooth_vals[val].current_value = start_val;
  vvv_smooth_vals[val].target_value = start_target;
  vvv_smooth_vals[val].min_value = min;
  vvv_smooth_vals[val].max_value = max;
  vvv_smooth_vals[val].smooth_param = param;
}

// update all smoothed values
bool vvv_smooth_update(void) {
  if (timer_elapsed(vvv_smooth_last_update) > VVV_SMOOTH_INTERVAL) {
    vvv_smooth_last_update = timer_read();
    for (uint8_t valIdx = 0; valIdx < VVV_SMOOTH_NVALS; valIdx++) {
      vvv_smooth_vals[valIdx].current_value = vvv_smooth_vals[valIdx].current_value * (1-vvv_smooth_vals[valIdx].smooth_param) +
                                              vvv_smooth_vals[valIdx].target_value * vvv_smooth_vals[valIdx].smooth_param;
    }
    return true;
  }
  return false;
}

void vvv_smooth_add_target(uint8_t val, int add) {
  if (val >= VVV_SMOOTH_NVALS) return;
  int new_val = vvv_smooth_vals[val].target_value + add;
  if (new_val > vvv_smooth_vals[val].max_value) vvv_smooth_vals[val].target_value = vvv_smooth_vals[val].max_value;
  else if (new_val < vvv_smooth_vals[val].min_value) vvv_smooth_vals[val].target_value = vvv_smooth_vals[val].min_value;
  else vvv_smooth_vals[val].target_value = (uint8_t) new_val;
}

void vvv_smooth_set_target(uint8_t val, uint8_t target) {
  if (val >= VVV_SMOOTH_NVALS) return;
  if (target > vvv_smooth_vals[val].max_value) vvv_smooth_vals[val].target_value = vvv_smooth_vals[val].max_value;
  else if (target < vvv_smooth_vals[val].min_value) vvv_smooth_vals[val].target_value = vvv_smooth_vals[val].min_value;
  else vvv_smooth_vals[val].target_value = target;
}

uint8_t vvv_get_val(uint8_t val) {
  if (val >= VVV_SMOOTH_NVALS) return 0;
  return vvv_smooth_vals[val].current_value;
}
