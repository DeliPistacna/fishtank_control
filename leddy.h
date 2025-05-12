#ifndef LEDDY_H
#define LEDDY_H

#define LEDDY_URL "http://192.168.1.248"
#define POWER_ON "/cm?cmnd=Power%20On"
#define POWER_OFF "/cm?cmnd=Power%20Off"
#define LEDDY_STATE_FILE ".leddy_state"
#define RESET_DELAY_MS 5000
#define DELAY_MS 10

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

void leddy_init();
LightState load_light_state();
void save_light_state();

LightState atols(char *ls);
const char *lstoa(LightState ls);

LightState state_after_cycles(int cycles);
int count_cycles_to_state(LightState ls);

void switch_state(LightState ls);
void state_reset();

// Curl
char *construct_url(const char *command);
void send_command(const char *command);

// Power
void power_off();
void power_on();
void power_reset();
void power_cycle(int cycles);

// Misc
void sleep_milliseconds(long milliseconds);

#endif // !LEDDY_H
