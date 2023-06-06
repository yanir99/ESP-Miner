idf_component_register(SRCS
"adc.c"
"DS4432U.c"
"EMC2101.c"
"fonts.c"
"INA260.c"
"led_controller.c"
"miner.c"
"oled.c"
"system.c"
"work_queue.c"
INCLUDE_DIRS "."
REQUIRES nvs_flash connect stratum esp_adc_cal bm1397
)
