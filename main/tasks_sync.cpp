
#include "tasks_sync.h"
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_event_group;

bool esp_wait_for_connection(const int BITS_TO_CHECK, const TickType_t xTicksToWait);

void task_sync_start()
{
	s_event_group = xEventGroupCreate();
	configASSERT( s_event_group );
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
	return esp_wait_for_connection(WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT, xTicksToWait);
#else
	return esp_wait_for_connection(WIFI_CONNECTED_BIT, xTicksToWait);
#endif
}

bool esp_wait_for_mqtt_connection(const TickType_t xTicksToWait)
{
	return esp_wait_for_connection(MQTT_CONNECTED_BIT, xTicksToWait);
}


bool esp_wait_for_connection(const int BITS_TO_CHECK, const TickType_t xTicksToWait)
{;
	#ifdef CONFIG_USE_WIFI_SMARTCONFIG
		EventBits_t uxBits = xEventGroupWaitBits(s_event_group, BITS_TO_CHECK, pdFALSE, pdFALSE, xTicksToWait);
	#else
		EventBits_t uxBits = xEventGroupWaitBits(s_event_group, BITS_TO_CHECK, pdFALSE, pdFALSE, xTicksToWait);
	#endif
		if(uxBits & BITS_TO_CHECK) {
			return true;
		}
		return false;
}
