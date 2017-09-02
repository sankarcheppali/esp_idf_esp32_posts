#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"

#define SSID "SSID"
#define PASSPHARSE "PASSWORD"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_EVENT = BIT0;
const int DISCONNECTED_EVENT = BIT1;



// Wifi event handler
static esp_err_t wifi_event_handler(void *context, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
	case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_EVENT);
        break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupSetBits(wifi_event_group, DISCONNECTED_EVENT);
        break;
	default:
        break;
    }
	return ESP_OK;
}

void printWiFiIP(void *pvParam){
    printf("printWiFiIP task started \n");
    while(1){
        xEventGroupWaitBits(wifi_event_group,CONNECTED_EVENT,true,true,portMAX_DELAY);
        tcpip_adapter_ip_info_t ip_info;
	    ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	    printf("IP :  %s\n", ip4addr_ntoa(&ip_info.ip));
    }
}
void printWiFiDiscntdMsg(void *pvParam){
    printf("printWiFiDiscntdMsg task started \n");
    while(1){
        xEventGroupWaitBits(wifi_event_group,DISCONNECTED_EVENT,true,true,portMAX_DELAY);
        printf("----WiFi disconnected from AP----\n");
    }
}
    
void app_main()
{	
    wifi_event_group = xEventGroupCreate();
    //Start the tasks
    xTaskCreate(&printWiFiIP,"printWiFiIP",2048,NULL,5,NULL);
    xTaskCreate(&printWiFiDiscntdMsg,"printWiFiDiscntdMsg",2048,NULL,5,NULL);

    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHARSE,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
	printf("connection intiated to %s\n", SSID);	
}
