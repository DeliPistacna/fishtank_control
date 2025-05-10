#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TASMOTA_URL "http://192.168.1.248"
#define POWER_ON "/cm?cmnd=Power%20On"
#define POWER_OFF "/cm?cmnd=Power%20Off"
#define DELAY_MS 50

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

void cycle_power(int cycles) {
  for (int i = 0; i < cycles; ++i) {
    power_off();
    power_on();
  }
}

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
    cycle_power(cycles);
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
    cycle_power(2);
  } else if (strcmp(argv[1], "night") == 0) {
    power_off();
    sleep_milliseconds(5000);
    cycle_power(3);
  } else {
    fprintf(stderr,
            "Invalid argument: %s. Use 'on' or 'off' or 'cycle <n>' or set "
            "light directly: day, daybreak, night.\n",
            argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
