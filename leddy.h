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

typedef enum {
  ON,
  OFF,
} POWER_STATE;

extern LightState states_cycle[];

extern LightState global_light_state;
extern POWER_STATE e_ps;

void leddy_init();
LightState get_stored_state();
void save_light_state();
LightState cycle_current_state();
const char *lstoa(LightState ls);
LightState atols(char *ls);
int count_cycles_to_state(LightState ls);
LightState state_after_cycles(int cycles);
void switch_state(LightState ls);
void state_reset();

char *construct_url(const char *command);
void sleep_milliseconds(long milliseconds);
void send_command(const char *command);
void power_off();
void power_on();
void power_reset();
void power_cycle(int cycles);

#endif // !LEDDY_H
