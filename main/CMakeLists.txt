idf_component_register(SRCS "main.c"
							"commands.c"
							"process_message.c"
							"remote_evt.c"
                        	"../../esp32_common/tcp/ntp_sync.c"
							"../../esp32_common/utils/utils.c"
							"../../esp32_common/cmds/cmd_wifi.c"
							"../../esp32_common/cmds/cmd_system.c"
							"../../esp32_common/tcp/tcp_log.c"
							"../../esp32_common/tcp/ntp_sync.c"
							"../../esp32_common/tcp/tcp_client.c"
							"../../esp32_common/nmea/nmea_parser.c"
							"../../esp32_common/adesp32/adc_op.c"
                    INCLUDE_DIRS "."
                    	"../certs"
                    	"../../esp32_common"
    					"../../esp32_common/cmds"
    					"../../esp32_common/utils"
    					"../../esp32_common/tcp"
    					"../../esp32_common/nmea"
    					"../../esp32_common/adesp32"
						)