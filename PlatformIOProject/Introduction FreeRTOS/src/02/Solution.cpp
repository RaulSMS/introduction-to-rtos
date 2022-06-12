/**
 * Solution to 02 - Blinky Challenge
 * 
 *  Original: Toggles LED at different rates using separate tasks.
 *  Modification: Writes over serial port to show in witch task the RTOS is
 * 
 * Date: June 12, 2022
 * Author: Raul Sainz
 * License: 0BSD
 */

#include <Arduino.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// TASK1
void SerialPrintTASK1(void *parameter) {
  while(1) {
    Serial.println("TASK1: Hello World!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("TASK1: Goodbye World!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
// TASK2
void SerialPrintTASK2(void *parameter) {
  while(1) {
    Serial.println("TASK2: Hello World!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println("TASK2: Goodbye World!");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {

  // Start serial port
  Serial.begin(115200);

  // Task to run forever
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              SerialPrintTASK1,    // Function to be called
              "TASK1", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL,         // Task handle
              app_cpu);     // Run on one core for demo purposes (ESP32 only)
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              SerialPrintTASK2,    // Function to be called
              "TASK2", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              NULL,         // Task handle
              app_cpu);     // Run on one core for demo purposes (ESP32 only)

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
}

void loop() {
  // Do nothing
  
  // setup() and loop() run in their own task with priority 1 in core 1
  // on ESP32
}
