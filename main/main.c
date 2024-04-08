/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;


#define adc_1 27
#define adc_2 26
#define adc_1_ID 1
#define adc_2_id 0



typedef struct adc {
    int axis;
    int val;
} adc_t;

void x_task(void *p){
    adc_init();
    adc_gpio_init(adc_1);
    
    while(true){
        adc_select_input(adc_1_ID);
    
        int result = adc_read();
        result=(result)*510/4095;
        result-=255;
        
        

        
        if(result<=140+10 && result>=110)
            result=0;
         
        adc_t x={0,result};
        
        xQueueSend(xQueueAdc,&x,portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));

    }
}
void y_task(void *p){
    adc_init();
    adc_gpio_init(adc_2);
    while(true){
        adc_select_input(adc_2_id);
    
        int result = adc_read();
        result-=2047;
        result=result*255/2047;
        
        
        
        
        if(result<=140+30 && result>=110)
            result=0;
       
        adc_t y={1,result};
        xQueueSend(xQueueAdc,&y,portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));

    }
}
void uart_task(void *p) {
    adc_t data;

    while (1) {
        if(xQueueReceive(xQueueAdc, &data, portMAX_DELAY)){;
            
            int val = data.val;
            
            int msb = val >> 8;
            int lsb = val & 0xFF ;

            uart_putc_raw(uart0, data.axis); 
            uart_putc_raw(uart0, lsb);
            uart_putc_raw(uart0, msb); 
            uart_putc_raw(uart0, -1); 
        }
    }
}


int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));
    
    xTaskCreate(x_task, "x_task", 4095, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4095, NULL, 1, NULL);
    
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}