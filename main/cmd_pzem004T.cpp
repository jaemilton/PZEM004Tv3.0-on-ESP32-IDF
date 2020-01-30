#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "driver/uart.h"
#include "tasks_sync.h"
#include "cmd_mqtt_client.h"
#include "cmd_pzem004T.h"


static uint8_t _contator_task = 0;
static TaskHandle_t TaskHandle[2];

static TickType_t s_xDelayMeansurePzem004T;

static task_data_t task_data_1;
#ifdef CONFIG_TWO_LINES
static task_data_t task_data_2;
#endif
void start_task_uart(const task_data_t * task_data);
string get_meansure_mqtt_message(const task_data_t * task_data, const power_meansuare_t* mensures);
void meansure_pzem004tv3_task(void *arg);
void print_meansures(const task_data_t * task_data, const power_meansuare_t * mensures);

void pzem004T_start()
{
	ESP_LOGI(TAG_PZEM004T, "Waiting for mqtt connection");
	if(esp_wait_for_mqtt_connection()) {
		//set meansures time to 2000ms
		set_pzem004T_update_delay(2000);

		ESP_LOGI(TAG_PZEM004T, "mqtt Connected");
		task_data_1 = {
			.task_name=	"L1",
			.mqtt_topic_name = CONFIG_BROKER_L1_TOPIC_NAME,
			.uart_data = {
				.uart_port = UART_NUM_1,
				.tx_io_num = CONFIG_UART_LINE_1_TX_PIN,
				.rx_io_num = CONFIG_UART_LINE_1_RX_PIN
			}
		  };
		  start_task_uart(&task_data_1);

		  #ifdef CONFIG_TWO_LINES
		  task_data_2 = {
			.task_name = "L2",
			.mqtt_topic_name = CONFIG_BROKER_L1_TOPIC_NAME,
			.uart_data = {
				.uart_port = UART_NUM_2,
				.tx_io_num = CONFIG_UART_LINE_2_TX_PIN,
				.rx_io_num = CONFIG_UART_LINE_2_RX_PIN
			}
		  };
		  start_task_uart(&task_data_2);
		  #endif
	}
}

#define STACK_SIZE 1024 *4

void start_task_uart(const task_data_t * task_data)
{
	uart_data_t uart_data = task_data->uart_data;
	TaskHandle_t  xHandle = NULL;
	ESP_LOGI(TAG_PZEM004T, "Variaveis da Task %s, UART: %d", task_data->task_name, uart_data.uart_port);
	xTaskCreate(meansure_pzem004tv3_task, task_data->task_name, STACK_SIZE, (void *) task_data, 10, &xHandle);
	configASSERT( xHandle );
	//stores the task handle to interact with then when necessary
	TaskHandle[_contator_task++] = xHandle;

	 // Use the handle to delete the task.
	if( xHandle != NULL )
	{
		ESP_LOGI(TAG_PZEM004T, "Iniciada a Task %s", task_data->task_name);
	}
}


string get_meansure_mqtt_message(const task_data_t * task_data, const power_meansuare_t* mensures)
{
	string message_mqtt;
	std::stringstream messagestream;
	messagestream << CONFIG_ESP_METER_ID << ";";
	messagestream << task_data->task_name << ";";
	messagestream << mensures->voltage << ";";
	messagestream << mensures->current  << ";";
	messagestream << mensures->power  << ";";
	messagestream << mensures->energy  << ";";
	messagestream << mensures->frequency  << ";";
	messagestream << mensures->pf  << ";";
	messagestream << mensures->alarms  << ";";
	messagestream >> message_mqtt;
	return message_mqtt;
}

void meansure_pzem004tv3_task(void *arg)
{
	const task_data_t * task_data = (const task_data_t *) arg;
	uart_data_t uart_data = task_data->uart_data;

	ESP_LOGI(TAG_PZEM004T, "Iniciando Task %s no Core %i na UART %i", task_data->task_name,  xPortGetCoreID(),  task_data->uart_data.uart_port);

#if CONFIG_DEBUG_ESP32
	ESP_LOGI(TAG_PZEM004T, "TopicName = %s", task_data->mqtt_topic_name);
#endif
	/* Use software serial for the PZEM*/
	PZEM004Tv30 pzem(&uart_data);
	bool pzem004_ajustado = false;
	const uint8_t addr = 0x42;

    while (1) {
    	if (!pzem004_ajustado)
    	{

    		ESP_LOGI(TAG_PZEM004T, "%s - Set address to 0x42", task_data->task_name );
    		pzem.setAddress(addr);
    		if (addr == pzem.getAddress())
    		{
				ESP_LOGI(TAG_PZEM004T, "%s - New Address %#.2x",task_data->task_name , pzem.getAddress());
				ESP_LOGI(TAG_PZEM004T, "%s - Reset Energy", task_data->task_name);
				pzem.resetEnergy();
				pzem004_ajustado = true;
    		}
    		else
    		{
    			ESP_LOGI(TAG_PZEM004T, "%s - Error to set New Address", task_data->task_name);
    			vTaskDelay( get_tick_type_for_milliseconds(1000) );
    		}
    	}

    	//Ensure that mqtt is connected
		if(pzem004_ajustado && esp_wait_for_mqtt_connection()) {
			power_meansuare_t * mensures = pzem.meansures();
			string meansuresMqttMessage = get_meansure_mqtt_message(task_data, mensures);
			mqtt_send_message(task_data->mqtt_topic_name, meansuresMqttMessage.c_str());
#ifdef CONFIG_DEBUG_ESP32
			print_meansures(task_data, mensures);
#endif
			vTaskDelay( s_xDelayMeansurePzem004T );
		}
		else
		{
			taskYIELD();
		}
	}

}

void print_meansures(const task_data_t * task_data, const power_meansuare_t * mensures)
{

	float voltage = mensures->voltage;
	if( !isnan(voltage) ){
		ESP_LOGI(TAG_PZEM004T, "%s - Voltage: %.1f V", task_data->task_name, voltage);
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading voltage", task_data->task_name);
	}

	float current = mensures->current;
	if( !isnan(current) ){
		ESP_LOGI(TAG_PZEM004T, "%s - Current: %.3f A", task_data->task_name, current);
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading current", task_data->task_name);
	}

	float power = mensures->power;
	if( !isnan(power) ){
		ESP_LOGI(TAG_PZEM004T, "%s - Power: %.1f W", task_data->task_name, power );
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading power",task_data->task_name);
	}

	float energy = mensures->energy;
	if( !isnan(energy) ){
		ESP_LOGI(TAG_PZEM004T, "%s - Energy: %.3f kWh",task_data->task_name, energy);
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading energy",task_data->task_name);
	}

	float frequency = mensures->frequency;
	if( !isnan(frequency) ){
		ESP_LOGI(TAG_PZEM004T, "%s - Frequency: %.1f Hz",task_data->task_name, frequency);
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading frequency", task_data->task_name);
	}

	float pf = mensures->pf;
	if( !isnan(pf) ){
		ESP_LOGI(TAG_PZEM004T, "%s - PF: %.2f",task_data->task_name, pf);
	} else {
		ESP_LOGI(TAG_PZEM004T, "%s - Error reading power factor",task_data->task_name);
	}

}

void set_pzem004T_update_delay(uint32_t milliseconds)
{
	s_xDelayMeansurePzem004T = get_tick_type_for_milliseconds(milliseconds);
}


