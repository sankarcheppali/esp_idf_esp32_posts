/*
program will change the pwm duty on every cycle

uses timer interrupts to load the next duty cycle configuration

*/

#include <stdio.h>
#include <stdlib.h>
#include <esp_types.h>
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_intr.h"
#include "esp_intr_alloc.h"
#include "soc/ledc_reg.h"
#include "soc/ledc_struct.h"
#include "freertos/semphr.h"

static char tag[] = "dynamic_duty";
void IRAM_ATTR ledc_tmr_ovf_isr(void* arg);
esp_err_t ledc_enable_tmr_ovf_intr(uint32_t channel);

static portMUX_TYPE ledc_spinlock = portMUX_INITIALIZER_UNLOCKED;
static uint32_t duties[] = {1000,1500,2000};
volatile uint32_t di=0; //duty index
volatile uint32_t counter = 0;

void IRAM_ATTR ledc_tmr_ovf_isr(void* arg){
	uint32_t intr_status = LEDC.int_st.val;  //read LEDC interrupt status.
    LEDC.int_clr.val = intr_status;  //clear LEDC interrupt status.
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duties[di]);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	di = di+1;
	if(di>2){
		di=0;
	}
	counter = counter + 1;
	
}

esp_err_t ledc_enable_tmr_ovf_intr(uint32_t channel)
{
    uint32_t value;
    portENTER_CRITICAL(&ledc_spinlock);
    value = LEDC.int_ena.val;
    uint8_t int_en_base = LEDC_HSTIMER0_OVF_INT_ENA_S;
    LEDC.int_ena.val = value | BIT(int_en_base + channel);
	
    portEXIT_CRITICAL(&ledc_spinlock);
    return ESP_OK;
}


void dynamicPWMDuty_task(void *ignore) {
	int bitSize         = 15;
	int minValue        = 500;  // micro seconds (uS)
	//int maxValue        = 2500; // micro seconds (uS)
	//int sweepDuration   = 1500; // milliseconds (ms)
	int duty            = (1<<bitSize) * minValue / 20000 ;//1638
	
	printf("in pwm task");
	ESP_LOGD(tag, ">> task_servo1");
	ledc_timer_config_t timer_conf;
	timer_conf.duty_resolution    = LEDC_TIMER_15_BIT;
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
	ledc_isr_register(ledc_tmr_ovf_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	ledc_enable_tmr_ovf_intr(ledc_conf.channel );
	printf("LEDC config\n");
	ledc_channel_config(&ledc_conf);
	ESP_LOGI(tag,"LEDC Setup done\n");

     do{
			vTaskDelay(1000/portTICK_PERIOD_MS);
			ESP_LOGI(tag,"?");
			if(counter > 250){
				counter  = 0 ;
				ESP_LOGI(tag,".");
			}
	}	while(bitSize==15); // End loop forever
	printf("LEDC Setup completed\n");
	vTaskDelete(NULL);
}


void app_main()
{	
    
        xTaskCreatePinnedToCore(&dynamicPWMDuty_task,"dynamicPWMDuty_task",4048,NULL,5,NULL,1);
		//xTaskCreate(&dynamicPWMDuty_task,"dynamicPWMDuty_task",4048,NULL,5,NULL);
        printf("DynamicPwmDuty task  started\n");
}
