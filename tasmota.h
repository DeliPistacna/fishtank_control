#ifndef TASMOTA_H
#define TASMOTA_H

#define TASMOTA_URL "http://192.168.1.248"
#define POWER_ON "/cm?cmnd=Power%20On"
#define POWER_OFF "/cm?cmnd=Power%20Off"
#define TASMOTA_STATE_FILE ".tasmota_state"
#define RESET_DELAY_MS 5000
#define DELAY_MS 10

typedef enum {
  DAY,
  DAYBREAK,
  NIGHT,
  UNKNOWN,
} LIGHT_STATE;

typedef enum {
  ON,
  OFF,
} POWER_STATE;

// typedef struct {
//
// } tasmota_commands;

extern LIGHT_STATE e_ls;
extern POWER_STATE e_ps;

void tasmota_init();
LIGHT_STATE get_stored_state();
void set_stored_state(LIGHT_STATE ls);
LIGHT_STATE cycle_current_state();
const char *lstoa(LIGHT_STATE ls);
LIGHT_STATE atols(char *ls);
int count_cycles_to_state(LIGHT_STATE ls);
LIGHT_STATE state_after_cycles(int cycles);
void switch_state(LIGHT_STATE ls);
void state_reset();

char *construct_url(const char *command);
void sleep_milliseconds(long milliseconds);
void send_command(const char *command);
void power_off();
void power_on();
void power_reset();
void power_cycle(int cycles);

#endif // !TASMOTA_H
