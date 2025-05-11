
#include "leddy.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  LightState state = LIGHT_STATE_DAY; // Default state
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

LightState get_stored_state() {
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
      power_reset();
      state = LIGHT_STATE_DAY;
    }
  }

  fclose(file);
  return state;
}

void save_light_state() {
  FILE *file = fopen(LEDDY_STATE_FILE, "w");
  if (file == NULL) {
    fprintf(stderr, "Could not open .leddy_state file\n");
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%s\n", lstoa(global_light_state));
  fclose(file);
}

void leddy_init() {
  global_light_state = get_stored_state();
  if (global_light_state == LIGHT_STATE_UNKNOWN) {
    power_reset();
    global_light_state = LIGHT_STATE_DAY;
    save_light_state();
  }
}

int count_cycles_to_state(LightState ls) {
  int current_state_index, required_cycles = 0;
  for (int i = 0; i < sizeof(states_cycle) / sizeof(LightState); i++) {
    if (states_cycle[i] == global_light_state) {
      current_state_index = i;
      break;
    }
  }

  while (states_cycle[current_state_index] != ls) {
    current_state_index++;
    required_cycles++;
    if (current_state_index >= sizeof(states_cycle) / sizeof(LightState))
      current_state_index = 0;
  }

  return required_cycles;
}

LightState state_after_cycles(int cycles) {
  int current_state_index;
  for (int i = 0; i < sizeof(states_cycle) / sizeof(LightState); i++) {
    if (states_cycle[i] == global_light_state) {
      current_state_index = i;
      break;
    }
  }

  for (int i = 0; i < cycles; ++i) {
    current_state_index++;
    if (current_state_index >= sizeof(states_cycle) / sizeof(LightState))
      current_state_index = 0;
  }

  return states_cycle[current_state_index];
}

void switch_state(LightState ls) {
  int cycles = count_cycles_to_state(ls);
  power_cycle(cycles);
  global_light_state = ls;
  save_light_state();
}

void state_reset() {
  LightState target = global_light_state;
  power_reset();
  switch_state(target);
}

char *construct_url(const char *command) {
  // Allocate memory for the full URL
  size_t url_length =
      strlen(LEDDY_URL) + strlen(command) + 1; // +1 for null terminator
  char *url = malloc(url_length);
  if (url == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  strcpy(url, LEDDY_URL);
  strcat(url, command);
  return url;
}

void send_command(const char *command) {
  CURL *curl;
  CURLcode res;

  char *url = construct_url(command);

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
  } else {
    fprintf(stderr, "Failed to initialize CURL\n");
  }
  free(url);
}

void sleep_milliseconds(long milliseconds) {
  struct timespec req, rem;
  req.tv_sec = milliseconds / 1000;              // Convert to seconds
  req.tv_nsec = (milliseconds % 1000) * 1000000; // Convert to nanoseconds
  nanosleep(&req, &rem);                         // Sleep for the specified time
}

void power_on() {
  send_command(POWER_ON);
  sleep_milliseconds(DELAY_MS);
}

void power_off() {
  send_command(POWER_OFF);
  sleep_milliseconds(DELAY_MS);
}

void power_reset() {
  power_off();
  sleep_milliseconds(RESET_DELAY_MS);
  power_on();
  global_light_state = LIGHT_STATE_DAY;
}

void power_cycle(int cycles) {
  LightState after = state_after_cycles(cycles);
  for (int i = 0; i < cycles; ++i) {
    power_off();
    power_on();
  }
  global_light_state = after;
}
