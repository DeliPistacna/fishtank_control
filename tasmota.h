#ifndef TASMOTA_H
#define TASMOTA_H

#define TASMOTA_URL "http://192.168.1.248"
#define POWER_ON "/cm?cmnd=Power%20On"
#define POWER_OFF "/cm?cmnd=Power%20Off"
#define DELAY_MS 10

char *construct_url(const char *command);
void sleep_milliseconds(long milliseconds);
void send_command(const char *command);
void power_off();
void power_on();
void power_cycle(int cycles);

#endif // !TASMOTA_H
