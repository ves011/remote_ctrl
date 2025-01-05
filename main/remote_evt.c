/*
 * joystick.c
 *
 *  Created on: Jan 2, 2025
 *      Author: viorel_serbu
 */

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "hal/adc_types.h"
#include "esp_netif.h"
#include "driver/gptimer.h"
//#include "esp_wifi.h"
#include "esp_spiffs.h"
#include "math.h"
//#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "common_defines.h"
#include "gpios.h"
#include "adc_op.h"
#include "remote_evt.h"

static const char *TAG = "remote-evt";
QueueHandle_t remote_evt_queue;

static int prev_js_rval, prev_js_lval;

static void js_timer_callback(void* arg)
	{
	evt_message_t msg;
	msg.msg_type = JSTIMER_MSG;
	xQueueSendFromISR(remote_evt_queue, &msg, NULL);
	}

static void remote_poll_task(void *pvParameters)
	{
	evt_message_t msg;
	esp_timer_handle_t js_timer;
	const esp_timer_create_args_t js_timer_args = 
		{
        .callback = &js_timer_callback,
        .name = "js timer"
    	};
    ESP_ERROR_CHECK(esp_timer_create(&js_timer_args, &js_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(js_timer, JS_POLL_INT)); // every 20 msec
    prev_js_rval = prev_js_lval = 1;
	while(1)
		{
		xQueueReceive(remote_evt_queue, &msg, portMAX_DELAY);
		switch(msg.msg_type)
			{
			case JSTIMER_MSG:
				{
				int chn[2] = {JS_RIGHT_CHANNEL, JS_LEFT_CHANNEL};
				int jsval[2][JS_ADC_SAMPLES], jsl = 0, jsr = 0;
				
				adc_get_data(chn, 2, (int **)jsval, JS_ADC_SAMPLES);
				for(int i = 0; i < JS_ADC_SAMPLES - 2; i++)
					{
					//ignore first 2 samples
					jsr += jsval[0][i + 2];
					jsl += jsval[1][i + 2];
					}
				jsr /= (JS_ADC_SAMPLES - 2); jsl /= (JS_ADC_SAMPLES - 2);
				if(jsr == 0) jsr =1;
				if(jsl == 0) jsl = 1;

				float dr = 1. - (float)(abs(jsr - prev_js_rval)) / (float)prev_js_rval;
				float dl = 1. - (float)(abs(jsl - prev_js_lval)) / (float)prev_js_lval;

				if(dr < JS_THERESHOLD || dl < JS_THERESHOLD)
					{
					ESP_LOGI(TAG, "joystick change: %5d %5d / %5d %5d", jsr, prev_js_rval , jsl, prev_js_lval);
					prev_js_rval = jsr;	prev_js_lval = jsl;
					}
				
				break;
				}
			default:
				break;
			}
		}
	}
void init_remote_event()
	{
	remote_evt_queue = xQueueCreate(5, sizeof(evt_message_t));
	ESP_LOGE(TAG, "Starting remote poll task");
	if(xTaskCreate(remote_poll_task, "remote_poll_task", 4096, NULL, 5, NULL) != pdPASS)
		{
		ESP_LOGE(TAG, "Unable to start remote poll task");
		esp_restart();
		}
	}


