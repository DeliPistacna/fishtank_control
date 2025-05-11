#include "tasmota.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

  tasmota_init();

  // TODO: Implement tasmota command chaining / buffering

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <on|off>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "cycle") == 0) {
    if (argc < 2) {
      printf("No cycle count provided");
      return EXIT_FAILURE;
    }
    int cycles = atoi(argv[2]);
    LIGHT_STATE target = state_after_cycles(cycles);
    power_cycle(cycles);
    e_ls = target;
    set_stored_state(e_ls);
  } else if (strcmp(argv[1], "on") == 0) {
    power_on();
  } else if (strcmp(argv[1], "off") == 0) {
    power_off();
  } else if (strcmp(argv[1], "reset") == 0) {
    state_reset();
  } else if (strcmp(argv[1], "day") == 0) {
    switch_state(DAY);
  } else if (strcmp(argv[1], "daybreak") == 0) {
    switch_state(DAYBREAK);
  } else if (strcmp(argv[1], "night") == 0) {
    switch_state(NIGHT);
  } else {
    fprintf(stderr,
            "Invalid argument: %s. Use 'on' or 'off' or 'cycle <n>' or set "
            "light directly: day, daybreak, night.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
