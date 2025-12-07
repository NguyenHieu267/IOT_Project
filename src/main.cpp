#include "global.h"
#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "main_server_task.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);  // Give Serial time to initialize
  
  Serial.println("\n\n=====================================");
  Serial.println("🚀 YoloUNO IoT Device Starting...");
  Serial.println("=====================================\n");
  check_info_File(0);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 8192, NULL, 2, NULL);
  xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  xTaskCreate(tiny_ml_task, "Tiny ML Task" ,8192  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop()
{
  if (check_info_File(1))
  {
    // WiFi credentials saved - try to connect to STA mode
    if (Wifi_reconnect())
    {
      // WiFi connected - start/maintain webserver
      Webserver_reconnect();
      //CORE_IOT_reconnect();
    }
    else
    {
      // WiFi not connected - stop webserver
      Webserver_stop();
    }
  }
  else
  {
    // No WiFi credentials - AP mode is already started in check_info_File(0)
    // Keep webserver running in AP mode
    Webserver_reconnect();
  }
}