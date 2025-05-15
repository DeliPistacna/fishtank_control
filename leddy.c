
#include "leddy.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LightState global_light_state = LIGHT_STATE_UNKNOWN;
LightState states_cycle[] = {LIGHT_STATE_DAY, LIGHT_STATE_DAYBREAK,
                             LIGHT_STATE_NIGHT};

TasmotaCommandChain *create_tasmota_command_chain(void) {
  TasmotaCommandChain *tcc = malloc(sizeof(TasmotaCommandChain));
  if (!tcc)
    return NULL;
  tcc->items = 0;
  tcc->capacity = TCC_INIT_CAP;
  tcc->buffer = malloc(sizeof(char *) * tcc->capacity);
  if (!tcc->buffer) {
    free(tcc);
    return NULL;
  }
  return tcc;
}

int add_command_to_tcc(TasmotaCommandChain *tcc, char *command) {
  // Resize buffer if needed
  if (tcc->items >= tcc->capacity) {
    size_t new_cap = tcc->capacity * 2;
    char **new_buffer = realloc(tcc->buffer, sizeof(char *) * new_cap);
    if (!new_buffer)
      return -1;
    tcc->buffer = new_buffer;
    tcc->capacity = new_cap;
  }

  tcc->buffer[tcc->items] = strdup(command);
  if (!tcc->buffer[tcc->items])
    return -1;

  tcc->items++;

  return 1;
}

// Free chain
void free_tcc(TasmotaCommandChain *tcc) {
  if (!tcc)
    return;
  for (size_t i = 0; i < tcc->items; ++i) {
    free(tcc->buffer[i]);
  }
  free(tcc->buffer);
  free(tcc);
}

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

LightState load_light_state() {
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
    fprintf(stderr, "Could not open %s file\n", LEDDY_STATE_FILE);
    exit(EXIT_FAILURE);
  }
  fprintf(file, "%s\n", lstoa(global_light_state));
  fclose(file);
}

void leddy_init() {
  global_light_state = load_light_state();
  if (global_light_state == LIGHT_STATE_UNKNOWN) {
    power_reset();
    global_light_state = LIGHT_STATE_DAY;
    save_light_state();
  }
}

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
  size_t url_length = strlen(LEDDY_URL) + strlen(CMND_PATH) + strlen(command) +
                      1; // +1 for null terminator
  char *url = malloc(url_length);
  if (url == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  url[0] = '\0';
  strcpy(url, LEDDY_URL);
  strcat(url, CMND_PATH);
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

void get_request(const char *url) {
  CURL *curl;
  CURLcode res;

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
}

int execute_tcc(TasmotaCommandChain *tcc) {
  if (!tcc || tcc->items == 0)
    return -1;

  size_t total_len = 0;

  for (size_t i = 0; i < tcc->items; ++i) {
    total_len += strlen(tcc->buffer[i]) + 3; // +3 for separator or null
  }

  char *command_buffer = malloc(total_len);
  if (!command_buffer)
    return -1;

  command_buffer[0] = '\0';

  for (size_t i = 0; i < tcc->items; ++i) {
    strcat(command_buffer, tcc->buffer[i]);
    if (i < tcc->items - 1) {
      strcat(command_buffer, "%3b");
    }
  }

  char *url = construct_url(command_buffer);

  printf("COMMAND: %s\n", command_buffer);
  printf("URL: %s\n", url);
  get_request(url);
  free(url);
  free(command_buffer);
  free_tcc(tcc);
  return 1;
}

void sleep_milliseconds(long milliseconds) {
  struct timespec req, rem;
  req.tv_sec = milliseconds / 1000;              // Convert to seconds
  req.tv_nsec = (milliseconds % 1000) * 1000000; // Convert to nanoseconds
  nanosleep(&req, &rem);                         // Sleep for the specified time
}

void power_on() {
  TasmotaCommandChain *tcc = create_tasmota_command_chain();
  add_command_to_tcc(tcc, POWER_ON);
  execute_tcc(tcc);
  global_light_state = LIGHT_STATE_DAY;
  save_light_state();
}

void power_off() {
  TasmotaCommandChain *tcc = create_tasmota_command_chain();
  add_command_to_tcc(tcc, POWER_OFF);
  execute_tcc(tcc);
  global_light_state = LIGHT_STATE_UNKNOWN;
  save_light_state();
}

void power_reset() {
  TasmotaCommandChain *tcc = create_tasmota_command_chain();
  add_command_to_tcc(tcc, POWER_OFF);
  add_command_to_tcc(tcc, PAUSE_RESET);
  add_command_to_tcc(tcc, POWER_ON);
  execute_tcc(tcc);
  global_light_state = LIGHT_STATE_DAY;
  save_light_state();
}

void power_cycle(int cycles) {
  LightState after = state_after_cycles(cycles);
  TasmotaCommandChain *tcc = create_tasmota_command_chain();
  for (int i = 0; i < cycles; ++i) {
    add_command_to_tcc(tcc, POWER_OFF);
    add_command_to_tcc(tcc, PAUSE);
    add_command_to_tcc(tcc, POWER_ON);
    add_command_to_tcc(tcc, PAUSE);
  }
  execute_tcc(tcc);
  global_light_state = after;
}
