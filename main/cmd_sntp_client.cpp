#include "cmd_sntp_client.h"
void initialize_sntp(void);

void obtain_time(void)
{
	if (esp_wait_for_wifi_connection())
	{
       initialize_sntp();

		// wait for time to be set
		int retry = 0;
		const int retry_count = 10;
		while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
			ESP_LOGI(TAG_SNTP, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
		sntp_stop();

	}

}


void get_timeinfo(time_t *_timer,  tm * timeinfo)
{
	 // Set timezone to Brazil,Sao Paulo Standard Time
	//setenv("TZ", "BRST+3BRDT+2,M10.3.0,M2.3.0", 1);
	setenv("TZ", "BRST+3", 1);
	tzset();
	localtime_r(_timer, timeinfo);
}


struct tm get_now_timeinfo(){
	time_t now;
	time(&now);
	//datetime_t xxx;
	struct tm timeinfo;
	get_timeinfo(&now, &timeinfo);
	return timeinfo;
}

string get_time_str()
{
	char strftime_buf[15];
	struct tm timeinfo = get_now_timeinfo();
	strftime(strftime_buf, sizeof(strftime_buf), "%Y%m%d%H%M%S", &timeinfo);
	return string(strftime_buf);
}


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_SNTP, "Notification of a time synchronization event");
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG_SNTP, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}
