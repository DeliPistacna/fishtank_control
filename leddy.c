
#include "leddy.h"
#include "tasmota.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

LightState global_light_state = LIGHT_STATE_UNKNOWN;
LightState states_cycle[] = {LIGHT_STATE_DAY, LIGHT_STATE_DAYBREAK,
                             LIGHT_STATE_NIGHT};

const char *lstoa(LightState ls) {
  switch (ls) {
  case LIGHT_STATE_DAY:
    return "DAY";
  case LIGHT_STATE_DAYBREAK:
    return "DAYBREAK";
  case LIGHT_STATE_NIGHT:
    return "NIGHT";
  case LIGHT_STATE_UNKNOWN:
    return "UNKNOWN"; // Handle the UNKNOWN state
  default:
    return "INVALID"; // Handle unexpected values
  }
}

LightState atols(char *ls) {
  LightState state = LIGHT_STATE_UNKNOWN; // Default state
  if (strcmp(ls, "DAY") == 0) {
    state = LIGHT_STATE_DAY;
  } else if (strcmp(ls, "DAYBREAK") == 0) {
    state = LIGHT_STATE_DAYBREAK;
  } else if (strcmp(ls, "NIGHT") == 0) {
    state = LIGHT_STATE_NIGHT;
  } else {
    state = LIGHT_STATE_UNKNOWN;
  }
  return state;
}

LightState load_light_state(TasmotaCommandChain *tcc) {
  LightState state = LIGHT_STATE_DAY; // Default state
  FILE *file = fopen(LEDDY_STATE_FILE, "r");
  if (file == NULL) {
    return LIGHT_STATE_UNKNOWN;
  }

  char line[256];

  // Read the file line by line
  while (fgets(line, sizeof(line), file)) {
    // Remove newline character if present
    line[strcspn(line, "\n")] = 0;

    // Compare the line with expected states
    state = atols(line);
    if (state == LIGHT_STATE_UNKNOWN) {
      power_reset(tcc);
      state = LIGHT_STATE_DAY;
    }
  }

  fclose(file);
  return state;
}

void save_light_state() {
  FILE *file = fopen(LEDDY_STATE_FILE, "w");
  if (file == NULL) {
    fprintf(stderr, "Could not open %s file\n", LEDDY_STATE_FILE);
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%s\n", lstoa(global_light_state));
  fclose(file);
}

void leddy_init(TasmotaCommandChain *tcc) {
  global_light_state = load_light_state(tcc);
  if (global_light_state == LIGHT_STATE_UNKNOWN) {
    power_reset(tcc);
    global_light_state = LIGHT_STATE_DAY;
  }
}
void leddy_done() { save_light_state(); }

int find_light_state_index(LightState ls) {
  for (int i = 0; i < sizeof(states_cycle) / sizeof(LightState); i++) {
    if (states_cycle[i] == ls) {
      return i;
    }
  }
  return 0;
}

int count_cycles_to_state(LightState ls) {
  int current_state_index = find_light_state_index(global_light_state),
      required_cycles = 0;

  while (states_cycle[current_state_index] != ls) {
    current_state_index++;
    required_cycles++;
    if (current_state_index >= sizeof(states_cycle) / sizeof(LightState))
      current_state_index = 0;
  }

  return required_cycles;
}

LightState state_after_cycles(int cycles) {
  int current_state_index = find_light_state_index(global_light_state);

  for (int i = 0; i < cycles; ++i) {
    current_state_index++;
    if (current_state_index >= sizeof(states_cycle) / sizeof(LightState))
      current_state_index = 0;
  }

  return states_cycle[current_state_index];
}

void switch_state(LightState ls, TasmotaCommandChain *tcc) {
  int cycles = count_cycles_to_state(ls);
  power_cycle(cycles, tcc);
  global_light_state = ls;
}

void state_reset(TasmotaCommandChain *tcc) {
  LightState target = global_light_state;
  power_reset(tcc);
  switch_state(target, tcc);
}

void sleep_milliseconds(long milliseconds) {
  struct timespec req, rem;
  req.tv_sec = milliseconds / 1000;              // Convert to seconds
  req.tv_nsec = (milliseconds % 1000) * 1000000; // Convert to nanoseconds
  nanosleep(&req, &rem);                         // Sleep for the specified time
}

void power_on(TasmotaCommandChain *tcc) {
  add_command_to_tcc(tcc, POWER_ON);
  add_command_to_tcc(tcc, PAUSE_RESET);
  global_light_state = LIGHT_STATE_DAY;
}

void power_off(TasmotaCommandChain *tcc) {
  add_command_to_tcc(tcc, POWER_OFF);
  add_command_to_tcc(tcc, PAUSE_RESET);
  global_light_state = LIGHT_STATE_UNKNOWN;
}

void power_reset(TasmotaCommandChain *tcc) {
  add_command_to_tcc(tcc, POWER_OFF);
  add_command_to_tcc(tcc, PAUSE_RESET);
  add_command_to_tcc(tcc, POWER_ON);
  global_light_state = LIGHT_STATE_DAY;
}

void power_cycle(int cycles, TasmotaCommandChain *tcc) {
  LightState after = state_after_cycles(cycles);
  for (int i = 0; i < cycles; ++i) {
    add_command_to_tcc(tcc, POWER_OFF);
    add_command_to_tcc(tcc, PAUSE);
    add_command_to_tcc(tcc, POWER_ON);
    add_command_to_tcc(tcc, PAUSE);
  }
  global_light_state = after;
}
