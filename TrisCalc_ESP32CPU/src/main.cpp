// ------------------ FREERTOS CONFIG ------------------
#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#define CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID

#include <Arduino.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <SPI.h>
// ------------------ CONFIG ------------------
#define GFXDisplay
#define TextDisplay
#define DEBUG
// --------------------------------------------

//HardwareSerial gpu = HardwareSerial(1);
#ifdef TextDisplay
#define gpu Serial2
#endif
#ifdef GFXDisplay
#define gpu2 Serial
#endif

TaskHandle_t loopCPU0handle;
TaskHandle_t loopCPU1handle;
//TaskHandle_t tasks[25];

char tasklist[1024];
void taskmgr()
{
  vTaskList(tasklist);

  char tmp[1028];
  strcpy(tmp, "T");
  strcat(tmp, tasklist);
  gpu2.println(tmp);
}

char statTmp[64];
const TickType_t statusUpdateSpeed = pdMS_TO_TICKS(2500);
void printstats(void *pvParamaters)
{
  const char *pcName = pcTaskGetTaskName(NULL);
  for (;;)
  {
    strcpy("SC", statTmp);
    //BaseType_t _taskNum = uxTaskGetNumberOfTasks();
    //strcat(pcTaskGetTaskName(loopCPU1handle), statTmp);
    //strcat((char*)_taskNum, statTmp);
    gpu2.println(pcName);
    gpu2.println(statTmp);
    vTaskDelay(statusUpdateSpeed);
  }
  vTaskDelete(NULL);
}

void loopCPU0(void *pvParamaters)
{
  for (;;)
  {
    taskmgr();
    delay(1000);

    //gpu2.println("Hello!");
    //delay(5000);
  }
  vTaskDelete(NULL);
}

void loopCPU1(void *pvParameters)
{
  while (true)
  {
    while (gpu.available() > 0)
    {
      Serial.println(char(gpu.read()));
      Serial.println(gpu.readString());
      char bfr[501];
      memset(bfr, 0, 501);
      gpu.readBytesUntil('\n', bfr, 500);
      Serial.println(bfr);
    }
    while (gpu2.available() > 0)
    {
      gpu2.read();
    }
  }
  vTaskDelete(NULL);
}

void setup()
{
  //Serial.begin(9600);
  gpu2.begin(115200);
  gpu.begin(19200, SERIAL_8N1, 16, 17);
  gpu.setTimeout(50);
  gpu2.setTimeout(50);
//while(gpu.available() <= 0){}
#ifdef DEBUG
  gpu2.println(xPortGetCoreID());
#endif
  delay(1000);
  gpu.println('A');
  gpu2.println('A');
  delay(500);
  gpu.println("m1");
  xTaskCreatePinnedToCore(loopCPU1, "Loop CPU1", 4096, NULL, 0, &loopCPU1handle, 1);
  xTaskCreatePinnedToCore(loopCPU0, "Loop CPU0", 4096, NULL, 0, &loopCPU0handle, 0);
  //xTaskCreate(printstats, "Statprint", 1024, NULL, 0, NULL);
  //vTaskStartScheduler();
}

void loop()
{
}