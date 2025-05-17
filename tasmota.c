#include "tasmota.h"

void tcc_init() { tcc = create_tasmota_command_chain(); }

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

void free_tcc(TasmotaCommandChain *tcc) {
  if (!tcc)
    return;
  for (size_t i = 0; i < tcc->items; ++i) {
    free(tcc->buffer[i]);
  }
  free(tcc->buffer);
  free(tcc);
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
