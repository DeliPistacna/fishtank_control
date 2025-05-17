
#ifndef TASMOTA_H
#define TASMOTA_H
#include <curl/curl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEDDY_URL "http://192.168.1.248"
#define CMND_PATH "/cm?cmnd=Backlog%20"
#define CMND_SEP "%3B"
#define POWER_ON "Power%20On"
#define POWER_OFF "Power%20Off"
#define PAUSE "Delay%201"
#define PAUSE_RESET "Delay%2050"
#define RESET_DELAY_MS 50
#define TCC_INIT_CAP 16

typedef struct {
  char **buffer;   // Array of command strings
  size_t items;    // Number of commands added
  size_t capacity; // Allocated slots
} TasmotaCommandChain;

extern TasmotaCommandChain *tcc;

void tcc_init();
TasmotaCommandChain *create_tasmota_command_chain(void);
int add_command_to_tcc(TasmotaCommandChain *tcc, char *command);
int execute_tcc(TasmotaCommandChain *tcc);

// Curl
char *construct_url(const char *command);
void send_command(const char *command);
#endif // !TASMOTA
