#include "leddy.h"
#include "tasmota.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

LightState init_state;
TasmotaCommandChain *tcc;

// Function to be called when Ctrl+C is pressed
void handle_sigint(int sig) {
  TasmotaCommandChain *tcc = create_tasmota_command_chain();
  switch_state(init_state, tcc);
  execute_tcc(tcc);
  exit(0); // Exit the program
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);

  tcc_init();
  leddy_init(tcc);
  // tcc = create_tasmota_command_chain();
  init_state = global_light_state;

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
    power_cycle(cycles, tcc);
    save_light_state();
  } else if (strcmp(argv[1], "on") == 0) {
    power_on(tcc);
  } else if (strcmp(argv[1], "off") == 0) {
    power_off(tcc);
  } else if (strcmp(argv[1], "feed") == 0) {
    LightState current = global_light_state;
    TasmotaCommandChain *daybreak = create_tasmota_command_chain();
    switch_state(LIGHT_STATE_DAYBREAK, daybreak);
    execute_tcc(daybreak);
    int sleep_mins = 10;
    if (argc > 2)
      sleep_mins = atoi(argv[2]);
    sleep(sleep_mins * 60);
    switch_state(current, tcc);
  } else if (strcmp(argv[1], "reset") == 0) {
    state_reset(tcc);
  } else if (strcmp(argv[1], "day") == 0) {
    switch_state(LIGHT_STATE_DAY, tcc);
  } else if (strcmp(argv[1], "daybreak") == 0) {
    switch_state(LIGHT_STATE_DAYBREAK, tcc);
  } else if (strcmp(argv[1], "night") == 0) {
    switch_state(LIGHT_STATE_NIGHT, tcc);
  } else {
    fprintf(stderr,
            "Invalid argument: %s. Use 'on' or 'off' or 'cycle <n>' or set "
            "light directly: day, daybreak, night.\n"
            "Use reset to put leddy into known state\n"
            "Use feed for temp DAYBREAK, then return to original state\n",
            argv[1]);
    return EXIT_FAILURE;
  }
  leddy_done();
  execute_tcc(tcc);
  return EXIT_SUCCESS;
}
