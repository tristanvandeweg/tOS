#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_wifi.h>

#define BUF_SIZE 1024
TaskHandle_t uart0Handle;
const uart_port_t uart0_port = UART_NUM_0;
bool uart0_started = false;
const uart_config_t uart0_config = {.baud_rate = 115200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS, .rx_flow_ctrl_thresh = 122};
void uart0(void *pvParam)
{
    QueueHandle_t uart0_queue;
    uart_param_config(uart0_port, &uart0_config);
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);
    uart0_started = true;
    while (true)
    {
    }
    vTaskDelete(NULL);
}

void gfxGPUmgr(void *pvParam)
{
}

void generatecpuusage(void *pvParam)
{
    //TaskHandle_t IDLE0_Handle = xTaskGetIdleTaskHandleForCPU(0);
    //TaskHandle_t IDLE1_Handle = xTaskGetIdleTaskHandleForCPU(1);
    //TaskStatus_t IDLE0_Status;
    TaskStatus_t *SystemStatus;
    volatile UBaseType_t uxArraySize;
    uint32_t ulTotalRunTime, ulStatsAsPercentage, Idle0p, Idle1p;
    Idle0p = 0;
    Idle1p = 0;
    ulStatsAsPercentage = 0;

    while (true)
    {
        char tmp[12];
        char output[32];
        uxArraySize = uxTaskGetNumberOfTasks();
        SystemStatus = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
        if (SystemStatus != NULL)
        {
            uxTaskGetSystemState(SystemStatus, uxArraySize, &ulTotalRunTime);
            ulTotalRunTime /= 100UL;
            for (uint8_t i = 0; i < uxArraySize; i++)
            {
                if (strcmp(SystemStatus[i].pcTaskName, "IDLE0"))
                {
                    Idle0p = SystemStatus[i].ulRunTimeCounter / ulTotalRunTime;
                }
                else if (strcmp(SystemStatus[i].pcTaskName, "IDLE1"))
                {
                    Idle1p = SystemStatus[i].ulRunTimeCounter / ulTotalRunTime;
                }
            }
            ulStatsAsPercentage = 100 - Idle0p - Idle1p;
            strcpy(output, "SC");
            sprintf(tmp, "%ld", (long)ulStatsAsPercentage);
            strcat(output, tmp);
            strcat(output, "\n");
#ifdef DEBUG
            printf(output);
#endif
        }
        //vTaskGetInfo(IDLE0_Handle, &IDLE0_Status, pdTRUE, NULL);
        //vTaskGetInfo(IDLE1_Handle, &IDLE1_Status, pdTRUE, NULL);
        vTaskDelay(2500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void taskmgr(void *pvParam)
{
    while (true)
    {
        char tmp[1024];
        char taskList[1024];
        /*gpio_config_t taskmgr_conf;
    taskmgr_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    taskmgr_conf.pin_bit_mask = 4;
    taskmgr_conf.mode = GPIO_MODE_INPUT;
    taskmgr_conf.pull_up_en = 1;
    gpio_config(&taskmgr_conf);
    gpio_set_intr_type(4, GPIO_INTR_POSEDGE);*/
        //printf("T");
        //printf("Name           State   Prio    Stack    PID\n");
        //printf("*****************************************\n");
        //printf(taskList);
        //printf("lol");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //tmp[0] = 0;
        vTaskList(tmp);
        //taskList[0] = 'T';
        strcpy(taskList, "T");
        strcat(taskList, "Name           State   Prio    Stack    PID   Core\n*****************************************\n");
        strcat(taskList, tmp);
        //strcat("T", taskList);
        //strcpy(tmp, taskList);
        //printf("made it");
        strcat(taskList, "Name           Time    Percent\n*****************************************\n");
        vTaskGetRunTimeStats(tmp);
        strcat(taskList, tmp);
        /*if (uart0_started)
    {
        uart_write_bytes(uart0_port, (const char *)taskList, strlen(taskList));
    }*/
#ifdef DEBUG
        printf(taskList);
#endif
    }
    vTaskDelete(NULL);
}

TaskHandle_t loop0Handle;
void loop0(void *pvParam)
{
    //printf(pcTaskGetTaskName(NULL));
    while (true)
    {
        break;
    }
    vTaskDelete(NULL);
}

TaskHandle_t loop1Handle;
void loop1(void *pvParam)
{
    xTaskCreate(generatecpuusage, "CPU Monitor", 2048, NULL, 0, NULL);
    xTaskCreate(taskmgr, "Taskmgr", 16384, NULL, 0, NULL);
    while (true)
    {
        break;
    }
    vTaskDelete(NULL);
}

void app_main()
{
#ifdef DEBUG
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
    //printf("Starting up...");
#endif
    //xTaskCreate(uart0, "UART Port 0", 4096, NULL, 0, &uart0Handle);               // Start UART Process
    xTaskCreatePinnedToCore(loop0, "CPU0 loop", 16384, NULL, 0, &loop0Handle, 0); // Start the main loop for core 0
    xTaskCreatePinnedToCore(loop1, "CPU1 loop", 16384, NULL, 0, &loop1Handle, 1); // Start the main loop for core 1
}