#ifndef CMD_SNTP_CLIENT_H
#define CMD_SNTP_CLIENT_H
#include "tasks_sync.h"
#include "esp_sntp.h"

void obtain_time(void);
string get_time_str();
#endif // CMD_SNTP_CLIENT_H
