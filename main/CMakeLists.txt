set(PROJECT_NAME "power_meter")

idf_component_register(SRCS 
							"main.cpp" 
							"include/PZEM004Tv30.cpp"
							"include/DS1307RTC.cpp"
							"tasks_sync.cpp" 
							"cmd_mqtt_client.cpp" 
							"cmd_wifi_smartconfig.cpp" 
							"cmd_pzem004T.cpp" 
							"cmd_sntp_client.cpp"
                        INCLUDE_DIRS  "." include 
                        PRIV_REQUIRES 
                        	esp_netif 
                        	nvs_flash 
                        	mqtt)
