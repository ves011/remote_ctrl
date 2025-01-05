#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_system.h"
//#include "esp_spi_flash.h"
#include "spi_flash_mmap.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_console.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wear_levelling.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "linenoise/linenoise.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "esp_pm.h"
//#include "esp32/clk.h"
#include "common_defines.h"
#include "cmd_wifi.h"
#include "cmd_system.h"
#include "utils.h"
#include "tcp_log.h"
#include "ntp_sync.h"
#include "esp_ota_ops.h"
#include "hal/adc_types.h"
#include "gpios.h"

//#include "driver/adc.h"
//#include "esp_adc_cal.h"
#include "external_defs.h"
#include "wifi_credentials.h"
#include "ntp_sync.h"
#include "tcp_client.h"
#include "nmea_parser.h"
#include "commands.h"
#include "adc_op.h"
#include "remote_evt.h"

#define PROMPT_STR "TCPREMOTE"
#define CONFIG_STORE_HISTORY 1
#define CONFIG_CONSOLE_MAX_COMMAND_LINE_LENGTH	1024

static const char *TAG ="remote";

console_state_t console_state;

int restart_in_progress;
int controller_op_registered;

static void initialize_nvs(void)
	{
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
		{
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
		}
	ESP_ERROR_CHECK(err);
	}
	
void app_main(void)
	{
	restart_in_progress = 0;
    console_state = CONSOLE_OFF;
	setenv("TZ","EET-2EEST,M3.4.0/03,M10.4.0/04",1);
	spiffs_storage_check();
	initialize_nvs();
	controller_op_registered = 0;
	wifi_join(DEFAULT_SSID, DEFAULT_PASS, JOIN_TIMEOUT_MS);
	if(rw_console_state(PARAM_READ, &console_state) == ESP_FAIL)
		console_state = CONSOLE_ON;
	esp_log_set_vprintf(my_log_vprintf);
	sync_NTP_time();
	esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
	//register_tcp_server();
	//xTaskCreate(tcp_server, "TCP server", 8192, NULL, 5, NULL);
	
#ifdef WITH_CONSOLE
	esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = CONFIG_CONSOLE_MAX_COMMAND_LINE_LENGTH;


#if CONFIG_STORE_HISTORY
	//initialize_filesystem();
	repl_config.history_save_path = BASE_PATH HISTORY_FILE;
	//ESP_LOGI(TAG, "Command history enabled");
#else
	ESP_LOGI(TAG, "Command history disabled");
#endif
#endif
	esp_console_register_help_command();
	register_system();
	register_wifi();
	register_tcp_client();
	register_nmea();
	register_commands();
	register_ad();
	init_remote_event();
	//sync_NTP_time();
	controller_op_registered = 1;
	/*
	ESP_LOGE(TAG, "Starting tcp client task");
	if(xTaskCreate(tcp_client_task, "tcp_client task", 8192, NULL, 4, NULL) != pdPASS)
		{
		ESP_LOGE(TAG, "Unable to start tcp client task");
		esp_restart();
		}
	*/	
#ifdef WITH_CONSOLE
#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

#elif defined(CONFIG_ESP_CONSOLE_USB_CDC)
    esp_console_dev_usb_cdc_config_t hw_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&hw_config, &repl_config, &repl));

#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    repl_config.task_stack_size = 8192;
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));
    //ESP_LOGI(TAG, "console stack: %d", repl_config.task_stack_size);

#else
	#error Unsupported console type
#endif

	ESP_ERROR_CHECK(esp_console_start_repl(repl));
#endif   
	}
