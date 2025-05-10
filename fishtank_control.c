#include "tasmota.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
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
  } else if (strcmp(argv[1], "on") == 0) {
    power_on();
  } else if (strcmp(argv[1], "off") == 0) {
    power_off();
  } else if (strcmp(argv[1], "day") == 0) {
    power_off();
    sleep_milliseconds(5000);
    power_on();
  } else if (strcmp(argv[1], "daybreak") == 0) {
    power_off();
    sleep_milliseconds(5000);
    power_cycle(2);
  } else if (strcmp(argv[1], "night") == 0) {
    power_off();
    sleep_milliseconds(5000);
    power_cycle(3);
  } else {
    fprintf(stderr,
            "Invalid argument: %s. Use 'on' or 'off' or 'cycle <n>' or set "
            "light directly: day, daybreak, night.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
