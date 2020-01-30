#ifndef CMD_MQTT_CLIENT_H
#define CMD_MQTT_CLIENT_H
#include "tasks_sync.h"

void mqtt_app_start();
int mqtt_send_message(const char* topicName, const char* message);

#endif // CMD_MQTT_CLIENT_H
