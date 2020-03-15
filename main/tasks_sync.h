#ifndef TASK_SYNC_H
#define TASK_SYNC_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "PZEM004Tv30.h"
#include <iostream>
#include <sstream>
#include "sdkconfig.h"
using namespace std;

typedef struct
{
	char *task_name;
	char *mqtt_topic_name;
	uart_data_t uart_data;
} task_data_t;


/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int WIFI_CONNECTED_BIT = BIT0;
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
static const int ESPTOUCH_DONE_BIT = BIT1;
#endif
static const int MQTT_CONNECTED_BIT = BIT2;
static const char *TAG_MQTT = "mqtts";
static const char *TAG_WIFI = "Wifi";
static const char *TAG_MAIN = "main";
static const char *TAG_PZEM004T = "pzem004t";
static const char *TAG_TASKSYNC = "tasksync";
static const char *TAG_SNTP = "sntp";

#define STACK_SIZE 1024 *4

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
void task_sync_start();
EventGroupHandle_t get_event_group();
TickType_t get_tick_type_for_milliseconds(uint32_t milliseconds);
bool esp_wait_for_wifi_connection(const TickType_t xTicksToWait = portMAX_DELAY);
bool esp_wait_for_mqtt_connection(const TickType_t xTicksToWait = portMAX_DELAY);
EventBits_t esp_set_bits(const EventBits_t uxBitsToClear);
EventBits_t esp_clear_bits(const EventBits_t uxBitsToClear);
esp_mqtt_client_handle_t register_mqtt_handler(esp_mqtt_client_config_t mqtt_cfg);
esp_mqtt_client_handle_t get_mqtt_handler();
void meansure_pzem004tv3_task(void *arg);

#endif // TASK_SYNC_H
