
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// #include "protocol_examples_common.h"
#include "main.h"

#include "asic_result_task.h"
#include "asic_task.h"
#include "create_jobs_task.h"
#include "esp_netif.h"
#include "global_state.h"
#include "http_server.h"
#include "nvs_config.h"
#include "serial.h"
#include "stratum_task.h"
#include "user_input_task.h"

static GlobalState GLOBAL_STATE = {.extranonce_str = NULL, .extranonce_2_len = 0, .abandon_work = 0, .version_mask = 0};

static const char * TAG = "miner";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value = nvs_config_get_u16(NVS_CONFIG_ASIC_FREQ, CONFIG_ASIC_FREQUENCY);
    ESP_LOGI(TAG, "NVS_CONFIG_ASIC_FREQ %f", (float)GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value);

    GLOBAL_STATE.asic_count = nvs_config_get_u16(NVS_CONFIG_ASIC_COUNT, 1);
    ESP_LOGI(TAG, "NVS_CONFIG_ASIC_COUNT %f", (float)GLOBAL_STATE.asic_count);

    GLOBAL_STATE.voltage_domain = nvs_config_get_u16(NVS_CONFIG_VOLTAGE_DOMAIN, 1);
    ESP_LOGI(TAG, "NVS_CONFIG_VOLTAGE_DOMAIN %f", (float)GLOBAL_STATE.voltage_domain);

    GLOBAL_STATE.device_model_str = nvs_config_get_string(NVS_CONFIG_DEVICE_MODEL, "");
    if (strcmp(GLOBAL_STATE.device_model_str, "max") == 0) {
        ESP_LOGI(TAG, "DEVICE: Max");
        GLOBAL_STATE.device_model = DEVICE_MAX;
    } else if (strcmp(GLOBAL_STATE.device_model_str, "ultra") == 0) {
        ESP_LOGI(TAG, "DEVICE: Ultra");
        GLOBAL_STATE.device_model = DEVICE_ULTRA;
    } else if (strcmp(GLOBAL_STATE.device_model_str, "supra") == 0) {
        ESP_LOGI(TAG, "DEVICE: Supra");
        GLOBAL_STATE.device_model = DEVICE_SUPRA;
    } else if (strcmp(GLOBAL_STATE.device_model_str, "hex") == 0) {
        ESP_LOGI(TAG, "DEVICE: Hex");
        GLOBAL_STATE.device_model = DEVICE_HEX;
    } else {
        ESP_LOGE(TAG, "Invalid DEVICE model");
        // maybe should return here to now execute anything with a faulty device parameter !
    }
    GLOBAL_STATE.board_version = atoi(nvs_config_get_string(NVS_CONFIG_BOARD_VERSION, "000"));
    ESP_LOGI(TAG, "Found Device Model: %s", GLOBAL_STATE.device_model_str);
    ESP_LOGI(TAG, "Found Board Version: %d", GLOBAL_STATE.board_version);

    GLOBAL_STATE.asic_model_str = nvs_config_get_string(NVS_CONFIG_ASIC_MODEL, "");
    if (strcmp(GLOBAL_STATE.asic_model_str, "BM1366") == 0) {
        ESP_LOGI(TAG, "ASIC: BM1366");
        GLOBAL_STATE.asic_model = ASIC_BM1366;
        AsicFunctions ASIC_functions = {.init_fn = BM1366_init,
                                        .receive_result_fn = BM1366_proccess_work,
                                        .set_max_baud_fn = BM1366_set_max_baud,
                                        .set_difficulty_mask_fn = BM1366_set_job_difficulty_mask,
                                        .send_work_fn = BM1366_send_work};
        GLOBAL_STATE.asic_job_frequency_ms = BM1366_FULLSCAN_MS / (double) GLOBAL_STATE.asic_count;
        GLOBAL_STATE.initial_ASIC_difficulty = BM1366_INITIAL_DIFFICULTY;

        GLOBAL_STATE.ASIC_functions = ASIC_functions;
    } else if (strcmp(GLOBAL_STATE.asic_model_str, "BM1368") == 0) {
        ESP_LOGI(TAG, "ASIC: BM1368");
        GLOBAL_STATE.asic_model = ASIC_BM1368;
        AsicFunctions ASIC_functions = {.init_fn = BM1368_init,
                                        .receive_result_fn = BM1368_proccess_work,
                                        .set_max_baud_fn = BM1368_set_max_baud,
                                        .set_difficulty_mask_fn = BM1368_set_job_difficulty_mask,
                                        .send_work_fn = BM1368_send_work};

        uint64_t bm1368_hashrate = GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value * BM1368_CORE_COUNT * 1000000;
        GLOBAL_STATE.asic_job_frequency_ms = (((double) NONCE_SPACE / (double) bm1368_hashrate) * 1000) / (double) GLOBAL_STATE.asic_count;
        GLOBAL_STATE.initial_ASIC_difficulty = BM1368_INITIAL_DIFFICULTY;

        GLOBAL_STATE.ASIC_functions = ASIC_functions;
    } else if (strcmp(GLOBAL_STATE.asic_model_str, "BM1397") == 0) {
        ESP_LOGI(TAG, "ASIC: BM1397");
        GLOBAL_STATE.asic_model = ASIC_BM1397;
        AsicFunctions ASIC_functions = {.init_fn = BM1397_init,
                                        .receive_result_fn = BM1397_proccess_work,
                                        .set_max_baud_fn = BM1397_set_max_baud,
                                        .set_difficulty_mask_fn = BM1397_set_job_difficulty_mask,
                                        .send_work_fn = BM1397_send_work};

        uint64_t bm1397_hashrate = GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value * BM1397_CORE_COUNT * 1000000;
        GLOBAL_STATE.asic_job_frequency_ms = (((double) NONCE_SPACE / (double) bm1397_hashrate) * 1000) / (double) GLOBAL_STATE.asic_count;
        GLOBAL_STATE.initial_ASIC_difficulty = BM1397_INITIAL_DIFFICULTY;

        GLOBAL_STATE.ASIC_functions = ASIC_functions;
    } else {
        ESP_LOGE(TAG, "Invalid ASIC model");
        AsicFunctions ASIC_functions = {.init_fn = NULL,
                                        .receive_result_fn = NULL,
                                        .set_max_baud_fn = NULL,
                                        .set_difficulty_mask_fn = NULL,
                                        .send_work_fn = NULL};
        GLOBAL_STATE.ASIC_functions = ASIC_functions;
        // maybe should return here to now execute anything with a faulty device parameter !
    }

    bool is_max = GLOBAL_STATE.asic_model == ASIC_BM1397;
    uint64_t best_diff = nvs_config_get_u64(NVS_CONFIG_BEST_DIFF, 0);
    uint16_t should_self_test = nvs_config_get_u16(NVS_CONFIG_SELF_TEST, 0);
    if (should_self_test == 1 && !is_max && best_diff < 1) {
        self_test((void *) &GLOBAL_STATE);
        vTaskDelay(60 * 60 * 1000 / portTICK_PERIOD_MS);
    }

    xTaskCreate(SYSTEM_task, "SYSTEM_task", 4096, (void *) &GLOBAL_STATE, 3, NULL);

    ESP_LOGI(TAG, "Welcome to the bitaxe!");

    // pull the wifi credentials and hostname out of NVS
    char * wifi_ssid = nvs_config_get_string(NVS_CONFIG_WIFI_SSID, WIFI_SSID);
    char * wifi_pass = nvs_config_get_string(NVS_CONFIG_WIFI_PASS, WIFI_PASS);
    char * hostname  = nvs_config_get_string(NVS_CONFIG_HOSTNAME, HOSTNAME);

    // copy the wifi ssid to the global state
    strncpy(GLOBAL_STATE.SYSTEM_MODULE.ssid, wifi_ssid, 20);

    // init and connect to wifi
    wifi_init(wifi_ssid, wifi_pass, hostname);
    start_rest_server((void *) &GLOBAL_STATE);
    EventBits_t result_bits = wifi_connect();

    if (result_bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to SSID: %s", wifi_ssid);
        strncpy(GLOBAL_STATE.SYSTEM_MODULE.wifi_status, "Connected!", 20);
    } else if (result_bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID: %s", wifi_ssid);

        strncpy(GLOBAL_STATE.SYSTEM_MODULE.wifi_status, "Failed to connect", 20);
        // User might be trying to configure with AP, just chill here
        ESP_LOGI(TAG, "Finished, waiting for user input.");
        while (1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        strncpy(GLOBAL_STATE.SYSTEM_MODULE.wifi_status, "unexpected error", 20);
        // User might be trying to configure with AP, just chill here
        ESP_LOGI(TAG, "Finished, waiting for user input.");
        while (1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    free(wifi_ssid);
    free(wifi_pass);
    free(hostname);

    // set the startup_done flag
    GLOBAL_STATE.SYSTEM_MODULE.startup_done = true;

    xTaskCreate(USER_INPUT_task, "user input", 8192, (void *) &GLOBAL_STATE, 5, NULL);

    if (GLOBAL_STATE.board_version >= 300 && GLOBAL_STATE.board_version < 400) {
        // this is a HEX board
        ESP_LOGI(TAG, "Starting HEX power management");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        xTaskCreate(POWER_MANAGEMENT_HEX_task, "power mangement", 8192, (void *) &GLOBAL_STATE, 10, NULL);
    } else {
        // this is NOT a HEX board
        ESP_LOGI(TAG, "Starting BITAXE power management");
        xTaskCreate(POWER_MANAGEMENT_task, "power mangement", 8192, (void *) &GLOBAL_STATE, 10, NULL);
    }

    ESP_LOGI(TAG, "Starting init functions");
    if (GLOBAL_STATE.ASIC_functions.init_fn != NULL) {
        wifi_softap_off();

        queue_init(&GLOBAL_STATE.stratum_queue);
        queue_init(&GLOBAL_STATE.ASIC_jobs_queue);

        SERIAL_init();
        (*GLOBAL_STATE.ASIC_functions.init_fn)(GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value, GLOBAL_STATE.asic_count);
        SERIAL_set_baud((*GLOBAL_STATE.ASIC_functions.set_max_baud_fn)());
        SERIAL_clear_buffer();

        xTaskCreate(stratum_task, "stratum admin", 8192, (void *) &GLOBAL_STATE, 5, NULL);
        xTaskCreate(create_jobs_task, "stratum miner", 8192, (void *) &GLOBAL_STATE, 10, NULL);
        xTaskCreate(ASIC_task, "asic", 8192, (void *) &GLOBAL_STATE, 10, NULL);
        xTaskCreate(ASIC_result_task, "asic result", 8192, (void *) &GLOBAL_STATE, 15, NULL);
    }
}

void MINER_set_wifi_status(wifi_status_t status, uint16_t retry_count)
{
    if (status == WIFI_RETRYING) {
        snprintf(GLOBAL_STATE.SYSTEM_MODULE.wifi_status, 20, "Retrying: %d", retry_count);
        return;
    } else if (status == WIFI_CONNECT_FAILED) {
        snprintf(GLOBAL_STATE.SYSTEM_MODULE.wifi_status, 20, "Connect Failed!");
        return;
    }
}
