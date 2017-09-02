#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


QueueHandle_t  q=NULL;
void consumer_task(void *pvParameter)
{
    unsigned long counter;
    if(q == NULL){
        printf("Queue is not ready");
        return;
    }
    while(1){
        xQueueReceive(q,&counter,(TickType_t )(1000/portTICK_PERIOD_MS)); 
        printf("value received on queue: %lu \n",counter);
        vTaskDelay(500/portTICK_PERIOD_MS); //wait for 500 ms
    }
}

void producer_task(void *pvParameter){
    unsigned long counter=1;
    if(q == NULL){
        printf("Queue is not ready \n");
        return;
    }
    while(1){
         printf("value sent on queue: %lu \n",counter);
         xQueueSend(q,(void *)&counter,(TickType_t )0); // add the counter value to the queue
         counter++;
         vTaskDelay(1000/portTICK_PERIOD_MS); //wait for a second
    }
}

void app_main()
{	
    q=xQueueCreate(20,sizeof(unsigned long));
    if(q != NULL){
        printf("Queue is created\n");
        vTaskDelay(1000/portTICK_PERIOD_MS); //wait for a second
        xTaskCreate(&producer_task,"producer_task",2048,NULL,5,NULL);
        printf("producer task  started\n");
        xTaskCreate(&consumer_task,"consumer_task",2048,NULL,5,NULL);
        printf("consumer task  started\n");
    }else{
        printf("Queue creation failed");
    }    
}
