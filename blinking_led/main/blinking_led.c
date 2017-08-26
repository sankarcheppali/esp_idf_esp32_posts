#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


void blink_task(void *pvParameter)
{
    //GPIO_NUM_16 is G16 on board
    gpio_set_direction(GPIO_NUM_16,GPIO_MODE_OUTPUT);
    printf("Blinking LED on GPIO 16\n");
    int cnt=0;
    while(1) {
		gpio_set_level(GPIO_NUM_16,cnt%2);
        cnt++;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main()
{	
    xTaskCreate(&blink_task,"blink_task",1024,NULL,5,NULL);
    printf("blink task  started\n");
}
