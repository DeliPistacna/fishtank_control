#ifndef LEDDY_H
#define LEDDY_H

#include <stddef.h>
#define LEDDY_URL "http://192.168.1.248"
#define CMND_PATH "/cm?cmnd=Backlog%20"
#define CMND_SEP "%3B"
#define POWER_ON "Power%20On"
#define POWER_OFF "Power%20Off"
#define PAUSE "Delay%201"
#define LEDDY_STATE_FILE ".leddy_state"
#define RESET_DELAY_MS 5000
#define DELAY_MS 10
#define TCC_INIT_CAP 16

typedef enum {
  LIGHT_STATE_DAY,
  LIGHT_STATE_DAYBREAK,
  LIGHT_STATE_NIGHT,
  LIGHT_STATE_UNKNOWN,
} LightState;

typedef struct {
  char **buffer;   // Array of command strings
  size_t items;    // Number of commands added
  size_t capacity; // Allocated slots
} TasmotaCommandChain;

TasmotaCommandChain *create_tasmota_command_chain(void);
int add_command_to_tcc(TasmotaCommandChain *tcc, char *command);
int execute_tcc(TasmotaCommandChain *tcc);

// TODO: check power state from tasmota?
// typedef enum {
//   ON,
//   OFF,
// } POWER_STATE;

extern LightState states_cycle[];

extern LightState global_light_state;

void leddy_init(void);
LightState load_light_state(void);
void save_light_state(void);

LightState atols(char *ls);
const char *lstoa(LightState ls);

LightState state_after_cycles(int cycles);
int count_cycles_to_state(LightState ls);

void switch_state(LightState ls);
void state_reset(void);

// Curl
char *construct_url(const char *command);
void send_command(const char *command);

// Power
void power_off(void);
void power_on(void);
void power_reset(void);
void power_cycle(int cycles);

// Misc
void sleep_milliseconds(long milliseconds);

#endif // !LEDDY_H
