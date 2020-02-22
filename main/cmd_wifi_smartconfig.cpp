

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
#include "esp_smartconfig.h"
#endif
#include "cmd_wifi_smartconfig.h"

#ifdef CONFIG_USE_WIFI_SMARTCONFIG
static void smartconfig_example_task(void * parm);
#endif

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	EventGroupHandle_t s_event_group = get_event_group();
	configASSERT( s_event_group );

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
    	xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, arg);
#endif
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_clear_bits(WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT);
    	ESP_LOGI(TAG_WIFI, "Disconnected");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    	esp_set_bits(WIFI_CONNECTED_BIT);
    	ip_event_got_ip_t *evt = (ip_event_got_ip_t *)event_data;

    	char ip[16] = {0};
    	char mask[16] = {0};
    	char gw[16] = {0};
    	esp_ip4addr_ntoa(&evt->ip_info.ip, ip, sizeof(ip));
    	esp_ip4addr_ntoa(&evt->ip_info.netmask, mask, sizeof(mask));
    	esp_ip4addr_ntoa(&evt->ip_info.gw, gw, sizeof(gw));

    	ESP_LOGI(TAG_WIFI, "core %i >> sta ip %s, mask: %s, gw:%s", xPortGetCoreID(), ip, mask, gw);
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG_WIFI, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG_WIFI, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG_WIFI, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG_WIFI, "SSID:%s", ssid);
        ESP_LOGI(TAG_WIFI, "PASSWORD:%s", password);

        ESP_ERROR_CHECK( esp_wifi_disconnect() );
        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        ESP_ERROR_CHECK( esp_wifi_connect() );
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_event_group, ESPTOUCH_DONE_BIT);
        esp_set_bits(ESPTOUCH_DONE_BIT);
#endif
    }
}


#ifdef CONFIG_USE_WIFI_SMARTCONFIG
static void smartconfig_example_task(void * parm)
{
	ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        if(esp_wait_for_wifi_connection()) {
            ESP_LOGI(TAG_WIFI, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}
#endif


void wifi_start()
{
	EventGroupHandle_t s_event_group = get_event_group();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, (void*)s_event_group) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, (void*)s_event_group) );
#ifdef CONFIG_USE_WIFI_SMARTCONFIG
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, event_handler, (void*)s_event_group) );
#endif
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );

#ifndef CONFIG_USE_WIFI_SMARTCONFIG
    wifi_config_t wifi_config;

	bzero(&wifi_config, sizeof(wifi_config_t));
	memcpy(wifi_config.sta.ssid, CONFIG_ESP_WIFI_SSID, sizeof(CONFIG_ESP_WIFI_SSID));
	memcpy(wifi_config.sta.password, CONFIG_ESP_WIFI_PASSWORD, sizeof(CONFIG_ESP_WIFI_PASSWORD));

	ESP_ERROR_CHECK( esp_wifi_disconnect() );
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK( esp_wifi_connect() );

	ESP_LOGI(TAG_WIFI, "Connecting to %s...", wifi_config.sta.ssid);

#endif
}


