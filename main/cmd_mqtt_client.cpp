#include <stdio.h>
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_tls.h"
#include "cmd_mqtt_client.h"
#include "cmd_pzem004T.h"

static esp_mqtt_client_handle_t s_mqtt_client = NULL;


#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipse_org_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipse_org_pem_start[]   asm("_binary_mqtt_server_pem_start");
#endif
extern const uint8_t mqtt_eclipse_org_pem_end[]   asm("_binary_mqtt_server_pem_end");

int mqtt_send_message(const char* topicName, const char* message)
{
	int msg_id = 0;
	if(esp_wait_for_wifi_connection()) {
		msg_id = esp_mqtt_client_publish(s_mqtt_client, topicName, message, strlen(message), 0, 0);
		ESP_LOGI(TAG_MQTT, "binary sent with msg_id=%d", msg_id);
	}
    return msg_id;
}



static esp_err_t mqtt_event_handler_cb(void* args, esp_mqtt_event_handle_t event)
{
	EventGroupHandle_t s_event_group = get_event_group();
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
        	xEventGroupSetBits(s_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/delaytime", 0);
            ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

            	break;
        case MQTT_EVENT_DISCONNECTED:
        	//Try to reconnect on case of disconnection
        	esp_mqtt_client_start(s_mqtt_client);
        	xEventGroupClearBits(s_event_group, MQTT_CONNECTED_BIT);
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            if (strncmp(event->topic, "/topic/delaytime", event->topic_len) == 0) {
				ESP_LOGI(TAG_MQTT, "Updating delaytime");
				set_pzem004T_update_delay(static_cast<uint32_t>(std::stoul(event->data)));
			}
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS) {
                ESP_LOGI(TAG_MQTT, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(TAG_MQTT, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG_MQTT, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(TAG_MQTT, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;
        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(handler_args, (esp_mqtt_event_handle_t)event_data);
}

void mqtt_app_start()
{
	ESP_LOGI(TAG_MQTT, "Waiting for wifi connection");
	if(esp_wait_for_wifi_connection()) {
		ESP_LOGI(TAG_MQTT, "WiFi Connected");
		const esp_mqtt_client_config_t mqtt_cfg = {
				.uri = CONFIG_BROKER_URI,
				.cert_pem = (const char *)mqtt_eclipse_org_pem_start
			};

		//ESP_LOGI(TAG_MQTT, "Server: %s, Cert: %s", mqtt_cfg.uri, mqtt_cfg.cert_pem);
		ESP_LOGI(TAG_MQTT, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
		s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_SUBSCRIBED, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_UNSUBSCRIBED, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_PUBLISHED, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_DATA, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_mqtt_client, MQTT_EVENT_BEFORE_CONNECT, mqtt_event_handler, NULL));
		ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));
	}

}

