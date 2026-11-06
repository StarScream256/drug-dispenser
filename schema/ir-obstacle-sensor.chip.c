#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t pin_out;
  uint32_t  attr_blocked;
} chip_state_t;

static void chip_timer_callback(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  
  // Read the value from the UI slider (0 or 1)
  uint32_t is_blocked = attr_read(chip->attr_blocked);
  
  if (is_blocked == 1) {
    // Obstacle present: Standard IR modules pull output LOW
    pin_write(chip->pin_out, LOW);
  } else {
    // Path clear: Output remains HIGH
    pin_write(chip->pin_out, HIGH);
  }
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  // Initialize the digital output pin
  chip->pin_out = pin_init("OUT", OUTPUT);
  
  // Bind to the control ID defined in the JSON file
  chip->attr_blocked = attr_init("blocked", 0);

  // Set up a microsecond timer to poll the slider state periodically
  const timer_config_t config = {
    .callback = chip_timer_callback,
    .user_data = chip,
  };
  timer_t timer_id = timer_init(&config);
  timer_start(timer_id, 1000, true); // Polls every 1000 microseconds (1ms)
}
