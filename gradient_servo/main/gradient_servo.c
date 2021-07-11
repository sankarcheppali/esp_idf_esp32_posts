/*
Connect servo to GPIO 23
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define SERVO_GPIO GPIO_NUM_23
#define SERVO_MIN_DUTY 900  // micro seconds (uS), for 0
#define SERVO_MAX_DUTY  3800 // micro seconds (uS),for 180
#define SERVO_TRANSITION_TIME 500 // in ms
#define SERVO_TIME_PERIOD 10000 // in ms
#define ACTIVE_ANGLE 180
#define REST_ANGLE 0

char *tag = "gradientServo";
int servo_duty = SERVO_MIN_DUTY ; 
int servo_delta = SERVO_MAX_DUTY - SERVO_MIN_DUTY;

void configureServo(){	
	ledc_timer_config_t timer_conf;
	timer_conf.duty_resolution    = LEDC_TIMER_15_BIT;
	timer_conf.freq_hz    = 50;
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	timer_conf.timer_num  = LEDC_TIMER_2;
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = LEDC_CHANNEL_2;
	ledc_conf.duty       = servo_duty;
	ledc_conf.gpio_num   = SERVO_GPIO;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = LEDC_TIMER_2;
	ledc_channel_config(&ledc_conf);
	ledc_fade_func_install(0);
}

void setServoAngle(int target_angle){
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, (uint16_t)(servo_duty+ (servo_delta*(target_angle/180.0))),SERVO_TRANSITION_TIME);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2,LEDC_FADE_WAIT_DONE);
}

void gradientServoTask(void *ignore) {
	 configureServo();
	 while(1){
            setServoAngle(ACTIVE_ANGLE);
            ESP_LOGI(tag,"Servo in active angle");
            vTaskDelay(pdMS_TO_TICKS(SERVO_TIME_PERIOD));
            setServoAngle(REST_ANGLE);
            ESP_LOGI(tag,"Servo in rest angle");
            vTaskDelay(pdMS_TO_TICKS(SERVO_TIME_PERIOD));
    }
}

void app_main()
{	
    xTaskCreate(&gradientServoTask,"servo_task",2048,NULL,5,NULL);
    printf("servo_task started");
}
