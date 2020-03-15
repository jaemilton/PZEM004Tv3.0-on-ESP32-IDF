
#include "tasks_sync.h"
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_event_group;
static TaskHandle_t  s_sync_task_handler = NULL;
static esp_mqtt_client_handle_t s_mqtt_client_handler = NULL;

bool esp_wait_for_bits(const int BITS_TO_CHECK, const TickType_t xTicksToWait);

void task_sync_start()
{
	s_event_group = xEventGroupCreate();
	configASSERT( s_event_group );

	ESP_LOGI(TAG_TASKSYNC, "Iniciando tarefa de sincronismo.");
	xTaskCreate(tasks_sync_task, "task_sync", STACK_SIZE, NULL, 10, &s_sync_task_handler);
	configASSERT( s_sync_task_handler );

}

EventGroupHandle_t get_event_group()
{
	return s_event_group;
}

TickType_t get_tick_type_for_milliseconds(uint32_t milliseconds)
{
	return milliseconds/ portTICK_PERIOD_MS;
}

bool esp_wait_for_wifi_connection(const TickType_t xTicksToWait)
{
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
	return esp_wait_for_bits(WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT, xTicksToWait);
#else
	return esp_wait_for_bits(WIFI_CONNECTED_BIT, xTicksToWait);
#endif
}

bool esp_wait_for_mqtt_connection(const TickType_t xTicksToWait)
{
	return esp_wait_for_bits(MQTT_CONNECTED_BIT | WIFI_CONNECTED_BIT, xTicksToWait);
}


bool esp_wait_for_bits(const int BITS_TO_CHECK, const TickType_t xTicksToWait)
{
	EventBits_t uxBits = xEventGroupWaitBits(s_event_group, BITS_TO_CHECK, pdFALSE, pdFALSE, xTicksToWait);
	if(uxBits & BITS_TO_CHECK) {
		return true;
	}
	return false;
}

EventBits_t esp_set_bits(const EventBits_t uxBitsToClear)
{
	return xEventGroupSetBits(s_event_group, uxBitsToClear);
}

EventBits_t esp_clear_bits(const EventBits_t uxBitsToSet)
{
	return xEventGroupClearBits(s_event_group, uxBitsToSet);
}

esp_mqtt_client_handle_t register_mqtt_handler(esp_mqtt_client_config_t mqtt_cfg)
{
	s_mqtt_client_handler = esp_mqtt_client_init(&mqtt_cfg);
	return get_mqtt_handler();
}

esp_mqtt_client_handle_t get_mqtt_handler()
{
	return s_mqtt_client_handler;
}


void tasks_sync_task(void *arg)
{
	while (1) {
		if (!esp_wait_for_mqtt_connection())
		{
			wifi_start();
		}
		else
		{

		}

	}
}

