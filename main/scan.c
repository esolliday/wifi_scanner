#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MAX_APs 20
static const char *TAG = "scan";

// from auth_mode code to string
static char* getAuthModeName(wifi_auth_mode_t auth_mode)
{
	char* names[] = {"OPEN", "WEP", "WPA PSK", "WPA WPA2 PSK", "MAX"};
	return names[auth_mode];
}

// empty event handler
static esp_err_t event_handler(void* ctx, system_event_t* event)
{
	return ESP_OK;
}

// empty infinite task
void loop_task(void* pvParameter)
{
	while(1)
	{
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}
static void wifi_scan(void)
{
	// initialize the wifi stack and event loop
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);

	// initialize the wifi in station mode
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// get the list of APs found in the last scan
	uint16_t ap_num = MAX_APs;
	wifi_ap_record_t ap_records[MAX_APs];
    uint16_t ap_count = 0;
    memset(ap_records, 0, sizeof(ap_records));

    // start the wifi and perform the scan
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

	printf("start scanning...");
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	printf("completed\n");
	printf("\n");

    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);

	// print the list
	printf("Found %d access points:\n", ap_num);
	printf("\n");
	printf("               SSID              | Channel | RSSI |   Auth Mode \n");
	printf("----------------------------------------------------------------\n");

	for( int i = 0; i < ap_num; i++)
	{
		printf("%32s | %7d | %4d | %12s\n", (char*)ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi, getAuthModeName(ap_records[i].authmode));
	}
	printf("----------------------------------------------------------------\n");
}

void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_scan();

	// infinite loop
	xTaskCreate(&loop_task, "loop_task", 2048, NULL, 5, NULL);
}

