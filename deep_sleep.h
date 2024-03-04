#ifndef DEEP_SLEEP_H_
#define DEEP_SLEEP_H_
#include "esp_sleep.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

void print_wakeup_reason(void);
// void deep_sleep_setup();
void deep_sleep_setup(const uint64_t sleep_time);

#endif





