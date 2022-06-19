/**
 * Solution to 04 - Heap Challenge
 *
 * One task reads from Serial, constructs a message buffer, and the second
 * prints the message to the console.
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

// defines
#define BUF_LEN 100
#define BUF_SIZE (BUF_LEN * sizeof(char))
#define LED_PIN 13

// Task handles
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

// Global variables
static const int led_pin = LED_PIN;
static char *buffer_ptr = NULL;
static volatile boolean is_done = false;

// TASK1 - Reads from Serial
void TASK1_ReadBuffer(void *parameter)
{
  char buff[BUF_LEN];
  int idx = 0;
  // Clear buffer
  memset(buff, 0, BUF_LEN);

  while (1)
  {
    if (Serial.available() > 0 && is_done == false)
    {
      // Read char
      char c = Serial.read();
      // Check buffer overflow
      if (idx < BUF_LEN - 1)
      {
        buff[idx] = c;
        idx++;
        Serial.println(buff);
      }
      else
      {
        Serial.println("Task 1: Buffer overflow.");
      }

      if (c == '\n')
      {
        Serial.println("Task 1: Received message: ");
        Serial.println(buff);
        buff[idx - 1] = '\0';
        // Allocate mmeory for buffer
        buffer_ptr = (char *)pvPortMalloc(idx * sizeof(char));
        // If malloc returns 0 (out of memory), throw an error and reset
        configASSERT(buffer_ptr);
        // copy buffer to new memory
        memcpy(buffer_ptr, buff, idx);

        // Reset control variables and buffer
        idx = 0;
        memset(buff, 0, BUF_LEN);
        is_done = true;
        // Print value of is_donen
        Serial.print("Task 1: is_done: ");
        Serial.println(is_done);
      }
    }
  }
}
// TASK2 - Writes to Serial
void TASK2_WriteBuffer(void *parameter)
{
  while (1)
  {
    if (is_done)
    {
      Serial.print("Task 2: Free heap: ");
      Serial.println(xPortGetFreeHeapSize());
      Serial.println("TAsk 2: Sending message: ");
      Serial.println(buffer_ptr);
      vPortFree(buffer_ptr);
      buffer_ptr = NULL;
      Serial.print("Task 2: Free heap: ");
      Serial.println(xPortGetFreeHeapSize());
      is_done = false;
    }
  }
}

void setup()
{

  // Start serial port
  Serial.begin(115200);

  // set led pin as output
  pinMode(led_pin, OUTPUT);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS 04 Solution---");
  Serial.println();
  Serial.println("Write a message to the serial port and press enter.");
  // Task to run forever
  xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
      TASK1_ReadBuffer,    // Function to be called
      "TASK1",             // Name of task
      (1024),              // Stack size (bytes in ESP32, words in FreeRTOS)
      NULL,                // Parameter to pass to function
      1,                   // Task priority (0 to configMAX_PRIORITIES - 1)
      &task_1,             // Task handle
      app_cpu);            // Run on one core for demo purposes (ESP32 only)
  xTaskCreatePinnedToCore( // Use xTaskCreate() in vanilla FreeRTOS
      TASK2_WriteBuffer,   // Function to be called
      "TASK2",             // Name of task
      (1024),              // Stack size (bytes in ESP32, words in FreeRTOS)
      NULL,                // Parameter to pass to function
      1,                   // Task priority (0 to configMAX_PRIORITIES - 1)
      &task_2,             // Task handle
      app_cpu);            // Run on one core for demo purposes (ESP32 only)

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() in
  // main after setting up your tasks.
}

void loop()
{
  // Keep alwasy an LED blinking
  digitalWrite(led_pin, HIGH);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  digitalWrite(led_pin, LOW);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  /*   Serial.print("Loop Task: Free heap: ");
    Serial.println(xPortGetFreeHeapSize()); */
}
