#include "leddy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  leddy_init();

  // TODO: Implement leddy command chaining / buffering

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
    power_cycle(cycles);
    save_light_state();
  } else if (strcmp(argv[1], "on") == 0) {
    power_on();
  } else if (strcmp(argv[1], "off") == 0) {
    power_off();
  } else if (strcmp(argv[1], "feed") == 0) {
    LightState current = global_light_state;
    switch_state(LIGHT_STATE_DAYBREAK);
    int sleep_mins = 10;
    if (argc > 2)
      sleep_mins = atoi(argv[2]);
    sleep(sleep_mins * 60);
    switch_state(current);
  } else if (strcmp(argv[1], "reset") == 0) {
    state_reset();
  } else if (strcmp(argv[1], "day") == 0) {
    switch_state(LIGHT_STATE_DAY);
  } else if (strcmp(argv[1], "daybreak") == 0) {
    switch_state(LIGHT_STATE_DAYBREAK);
  } else if (strcmp(argv[1], "night") == 0) {
    switch_state(LIGHT_STATE_NIGHT);
  } else {
    fprintf(stderr,
            "Invalid argument: %s. Use 'on' or 'off' or 'cycle <n>' or set "
            "light directly: day, daybreak, night.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
