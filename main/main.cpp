/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "nvs_flash.h"
#include "tasks_sync.h"
#include "cmd_wifi_smartconfig.h"
#include "cmd_mqtt_client.h"
#include "cmd_pzem004T.h"
#include "cmd_sntp_client.h"


const char * MAIN = "app_main";


extern "C" void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
	esp_log_level_set(TAG_MQTT, ESP_LOG_VERBOSE);
	esp_log_level_set(TAG_WIFI, ESP_LOG_VERBOSE);
	task_sync_start();
	ESP_LOGI(TAG_MAIN, "task_sync started");
	wifi_start();
	ESP_LOGI(TAG_MAIN, "wifi started");
	obtain_time();
	ESP_LOGI(TAG_MAIN, "sntp started");
	mqtt_app_start();
	ESP_LOGI(TAG_MAIN, "mqtt started");
	pzem004T_start();
	ESP_LOGI(TAG_MAIN, "pzem004T started");

}



