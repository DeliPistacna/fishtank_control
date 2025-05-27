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

void switch_state(LightState ls, TasmotaCommandChain *tcc);
void state_reset(TasmotaCommandChain *tcc);

// Power
void power_off(TasmotaCommandChain *tcc);
void power_on(TasmotaCommandChain *tcc);
void power_reset(TasmotaCommandChain *tcc);
void power_cycle(int cycles, TasmotaCommandChain *tcc);

#endif // !LEDDY_H
