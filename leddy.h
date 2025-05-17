#ifndef LEDDY_H
#define LEDDY_H

#include "tasmota.h"
#define LEDDY_STATE_FILE ".leddy_state"

typedef enum {
  LIGHT_STATE_DAY,
  LIGHT_STATE_DAYBREAK,
  LIGHT_STATE_NIGHT,
  LIGHT_STATE_UNKNOWN,
} LightState;

// TODO: check power state from tasmota?
// typedef enum {
//   ON,
//   OFF,
// } POWER_STATE;

extern LightState states_cycle[];
extern LightState global_light_state;

void leddy_init(TasmotaCommandChain *tcc);
void leddy_done(void);
LightState load_light_state(TasmotaCommandChain *tcc);
void save_light_state(void);

LightState atols(char *ls);
const char *lstoa(LightState ls);

LightState state_after_cycles(int cycles);
int count_cycles_to_state(LightState ls);

void switch_state(LightState ls, TasmotaCommandChain *tcc);
void state_reset(TasmotaCommandChain *tcc);

// Power
void power_off(TasmotaCommandChain *tcc);
void power_on(TasmotaCommandChain *tcc);
void power_reset(TasmotaCommandChain *tcc);
void power_cycle(int cycles, TasmotaCommandChain *tcc);

// Misc
void sleep_milliseconds(long milliseconds);

#endif // !LEDDY_H
