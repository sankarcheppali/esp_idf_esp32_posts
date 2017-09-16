#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#define SSID "DangerVirus"
#define PASSPHARSE "abivarsh@2016"
#define minValue 900  // micro seconds (uS), for 0
#define maxValue  3800 // micro seconds (uS),for 180

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


static char tag[] = "network_servo";
const int bitSize =15;
int delta = maxValue - minValue;;
int time_period = 20000; // micro
int duty;


void wifi_connect(){
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHARSE,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void configureServo(){	
    duty = minValue ; 
	ESP_LOGD(tag, ">> task_servo1");
	ledc_timer_config_t timer_conf;
	timer_conf.bit_num    = LEDC_TIMER_15_BIT;
	timer_conf.freq_hz    = 50;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = LEDC_TIMER_0;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.duty       = duty;
	ledc_conf.gpio_num   = 16;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_channel_config(&ledc_conf);
}

void setAngle(int target_angle){
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty+ (delta*(target_angle/180.0)) );
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0); 
}


void udp_server(void *pvParam){
    ESP_LOGI(tag,"udp_server task started \n");
    struct sockaddr_in udpServerAddr;
    udpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = htons( 3020 );
    int s, r;
    char recv_buf[64];
    static struct sockaddr_in remote_addr;
    static unsigned int socklen;
    socklen = sizeof(remote_addr);
    while(1){
        xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if(s < 0) {
            ESP_LOGE(tag, "... Failed to allocate socket.\n");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(tag, "... allocated socket\n");
         if(bind(s, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr)) != 0) {
            ESP_LOGE(tag, "... socket bind failed errno=%d \n", errno);
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(tag, "... socket is binded \n");
        do {
            bzero(recv_buf, sizeof(recv_buf));
            //r = recv(s, recv_buf, 4,0);// every record is 4 bytes long
            r=recvfrom(s, recv_buf, 64, 0, (struct sockaddr *)&remote_addr, &socklen);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
            int rcvdAngle=atoi(recv_buf);
            printf("\nAngle  :%d\n",rcvdAngle);
            setAngle(rcvdAngle);
        } while(r > 0);
        ESP_LOGI(tag, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        ESP_LOGI(tag, "... new sockeet will be opened in a second");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(tag, "...udp_server task closed\n");
}


void app_main()
{	
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();
    initialise_wifi();
    configureServo();
    xTaskCreate(&udp_server,"udp_server",4048,NULL,5,NULL);
    printf("network servo  started\n");
}


