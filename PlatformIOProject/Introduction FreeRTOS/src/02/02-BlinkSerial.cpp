/**
 * Demo for 02 - Blinky
 * 
 * Toggles the LED on and off in its own task/thread.
 * 
 * Date: December 3, 2020
 * Author: Shawn Hymel
 * License: 0BSD
 */

#include <Arduino.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


// Our task: blink an LED
void SerialPrint(void *parameter) {
  while(1) {
    Serial.println("Hello World!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Goodbye World!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {

  // Start serial port
  Serial.begin(115200);

  // Task to run forever
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              SerialPrint,    // Function to be called
              "Print on Serial Monitor", // Name of task
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
