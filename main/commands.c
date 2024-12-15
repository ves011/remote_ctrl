/*
 * commands.c
 *
 *  Created on: Dec 11, 2024
 *      Author: viorel_serbu
 */

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "hal/gpio_types.h"
#include "freertos/freertos.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "sys/stat.h"
#include "esp_timer.h"
#include "cmd_types.h"
#include "project_specific.h"
#include "common_defines.h"
#include "external_defs.h"
#include "commands.h"
#include "tcp_server.h"

static struct {
    struct arg_str *op;
    struct arg_int *arg1;
    struct arg_int *arg2;
    struct arg_end *end;
} st_args;

static const char* TAG = "cmds";

static int do_st(int argc, char **argv)
	{
	int nerrors = arg_parse(argc, argv, (void **)&st_args);
	int arg = INVALID_ARG;
	socket_message_t msg;
    if (nerrors != 0)
    	{
        arg_print_errors(stderr, st_args.end, argv[0]);
        return 1;
    	}
	
	if(st_args.arg1->count)
    	arg = st_args.arg1->ival[0];
	
	if(!strcmp(st_args.op->sval[0], "l"))
    	{
		if(tcp_send_queue && commstate == CONNECTED)
    		{
			msg.ts = esp_timer_get_time();
			msg.cmd_id = ST_LEFT;
			msg.p.u32params[0] = arg;
			int nmsg = uxQueueMessagesWaiting(tcp_send_queue);
			if(nmsg >= TCP_QUEUE_SIZE)
				{
				xQueueReset(tcp_send_queue);
				ESP_LOGI(TAG, "queue reset %d", nmsg);
				}
			xQueueSend(tcp_send_queue, &msg, 0);
			}
		}
	else if(!strcmp(st_args.op->sval[0], "r"))
    	{
		if(tcp_send_queue && commstate == CONNECTED)
    		{
			msg.ts = esp_timer_get_time();
			msg.cmd_id = ST_RIGHT;
			msg.p.u32params[0] = arg;
			int nmsg = uxQueueMessagesWaiting(tcp_send_queue);
			if(nmsg >= TCP_QUEUE_SIZE)
				{
				xQueueReset(tcp_send_queue);
				ESP_LOGI(TAG, "queue reset %d", nmsg);
				}
			xQueueSend(tcp_send_queue, &msg, 0);
			}
		}
	return 0;
	}

static void register_st()
	{
	st_args.op = arg_str1(NULL, NULL, "<op>", "l | r");
	st_args.arg1 = arg_int0(NULL, NULL, "<#angle(deg)>", "angle degree to turn");
	st_args.arg2 = arg_int0(NULL, NULL, "placeholder", "placeholder");
    st_args.end = arg_end(1);
    const esp_console_cmd_t st_cmd =
    	{
        .command = "st",
        .help = "steering commands",
        .hint = NULL,
        .func = &do_st,
        .argtable = &st_args
    	};
    ESP_ERROR_CHECK(esp_console_cmd_register(&st_cmd));
	}
	
void register_commands()
	{
	register_st();
	}

