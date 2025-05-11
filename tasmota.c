
#include "tasmota.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIGHT_STATE e_ls;
POWER_STATE e_ps;

const char *lstoa(LIGHT_STATE ls) {
  switch (ls) {
  case DAY:
    return "DAY";
  case DAYBREAK:
    return "DAYBREAK";
  case NIGHT:
    return "NIGHT";
  case UNKNOWN:
    return "UNKNOWN"; // Handle the UNKNOWN state
  default:
    return "INVALID"; // Handle unexpected values
  }
}

LIGHT_STATE atols(char *ls) {
  LIGHT_STATE state = DAY; // Default state
  if (strcmp(ls, "DAY") == 0) {
    state = DAY;
  } else if (strcmp(ls, "DAYBREAK") == 0) {
    state = DAYBREAK;
  } else if (strcmp(ls, "NIGHT") == 0) {
    state = NIGHT;
  } else {
    state = UNKNOWN;
  }
  return state;
}

LIGHT_STATE get_stored_state() {
  LIGHT_STATE state = DAY; // Default state
  FILE *file = fopen(TASMOTA_STATE_FILE, "r");
  if (file == NULL) {
    return UNKNOWN;
  }

  char line[256];

  // Read the file line by line
  while (fgets(line, sizeof(line), file)) {
    // Remove newline character if present
    line[strcspn(line, "\n")] = 0;

    // Compare the line with expected states
    state = atols(line);
    if (state == UNKNOWN) {
      power_reset();
      state = DAY;
    }
  }

  fclose(file);
  return state;
}

void set_stored_state(LIGHT_STATE ls) {
  FILE *file = fopen(TASMOTA_STATE_FILE, "w");
  if (file == NULL) {
    fprintf(stderr, "Could not open .tasmota_state file\n");
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%s\n", lstoa(ls));
  fclose(file);
}

void tasmota_init() {
  e_ls = get_stored_state();
  if (e_ls == UNKNOWN) {
    power_reset();
    e_ls = DAY;
    set_stored_state(e_ls);
  }
}

int count_cycles_to_state(LIGHT_STATE ls) {
  LIGHT_STATE states_cycle[] = {DAY, DAYBREAK, NIGHT};
  int current_state_index, required_cycles = 0;
  for (int i = 0; i < sizeof(states_cycle) / sizeof(LIGHT_STATE); i++) {
    if (states_cycle[i] == e_ls) {
      current_state_index = i;
      break;
    }
  }

  while (states_cycle[current_state_index] != ls) {
    current_state_index++;
    required_cycles++;
    if (current_state_index >= sizeof(states_cycle) / sizeof(LIGHT_STATE))
      current_state_index = 0;
  }

  return required_cycles;
}

void switch_state(LIGHT_STATE ls) {
  int cycles = count_cycles_to_state(ls);
  power_cycle(cycles);
  e_ls = ls;
  set_stored_state(e_ls);
}

void state_reset() {
  LIGHT_STATE target = e_ls;
  power_reset();
  e_ls = DAY;
  switch_state(target);
  set_stored_state(target);
}

char *construct_url(const char *command) {
  // Allocate memory for the full URL
  size_t url_length =
      strlen(TASMOTA_URL) + strlen(command) + 1; // +1 for null terminator
  char *url = malloc(url_length);
  if (url == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  strcpy(url, TASMOTA_URL);
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
}

void power_cycle(int cycles) {
  for (int i = 0; i < cycles; ++i) {
    power_off();
    power_on();
  }
}
