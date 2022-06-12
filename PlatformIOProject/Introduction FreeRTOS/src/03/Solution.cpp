/**
 * Solution to 03 - LED Challenge
 * 
 * One task flashes an LED at a rate specified by a value set in another task.
 * 
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

// Defines
#define External_LED_Pin 13

// Task handles
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

// Global variables
int Delay_Time = 1000;

// Pins
static const int led_pin = External_LED_Pin;

// TASK1 - Set LED blink delay time
void TASK1_BlinkLED(void *parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(Delay_Time / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(Delay_Time / portTICK_PERIOD_MS);
    }
}
// TASK2 - Listen on Serial Monitor for blink rate
void TASK2_SerialListen(void *parameter) {
  while(1) {
    if(Serial.available() > 0) {
      Delay_Time = Serial.parseInt();
      Serial.print("Delay time set to: ");
      Serial.println(Delay_Time);
      Serial.print("Input the delay time: ");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {

  // Start serial port
  Serial.begin(115200);
  // Configure Led PIn
  pinMode(led_pin, OUTPUT);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS 03 Solution---");
  Serial.print("Input the delay time: ");

  // Task to run forever
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              TASK1_BlinkLED,    // Function to be called
              "TASK1", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              &task_1,      // Task handle
              app_cpu);     // Run on one core for demo purposes (ESP32 only)
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
              TASK2_SerialListen,    // Function to be called
              "TASK2", // Name of task
              1024,         // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,         // Parameter to pass to function
              1,            // Task priority (0 to configMAX_PRIORITIES - 1)
              &task_2,      // Task handle
              app_cpu);     // Run on one core for demo purposes (ESP32 only)

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
}

void loop() {

}
