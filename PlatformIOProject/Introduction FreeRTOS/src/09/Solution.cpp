/**
 * ESP32 Sample and Process Solution
 * 
 * Sample ADC in an ISR, process in a task.
 * 
 * Date: February 3, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include <semphr.h>
#include <Arduino.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif
// Macros
#define MAX_ADC_BUFF_LEN  10
#define MSG_LEN           1000

// Settings
static const uint16_t timer_divider = 80;         // Divide 80 MHz by this -> 1 MHz
static const uint64_t timer_max_count = 100000;  // Timer counts to this value -> 100 ms
static const char command[] = "avg";

// Analog pin
static const int adc_pin = A4;

// Globals
static hw_timer_t *timer = NULL;
static volatile int AdcBuff[MAX_ADC_BUFF_LEN];
static volatile float AvgVal = 0.0;
static volatile int AdcBuffIndex = 0;
static SemaphoreHandle_t AverageSemaphore = NULL;
static const bool debug = false;
                   
//*****************************************************************************
// Functions that can be called from anywhere (in this file)

//*****************************************************************************
// Interrupt Service Routines (ISRs)

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer() {
  
  int val;
  
  if (AdcBuffIndex >= MAX_ADC_BUFF_LEN) {
    AdcBuffIndex = 0;
    // Release semaphore to calculate average
    xSemaphoreGive(AverageSemaphore);
    // debug print
    if (debug) {
    Serial.println("Releasing semaphore");
    // Print hole buffer
    for (int i = 0; i < MAX_ADC_BUFF_LEN; i++) {
      Serial.println(AdcBuff[i]);
    }
    }
  }
  // Read from Analog and save into buffer
  val = analogRead(adc_pin);
  AdcBuff[AdcBuffIndex] = val;
  AdcBuffIndex++;   
  //Debug print
  if (debug) {
    Serial.print("ADC Value: ");
    Serial.println(val);
  }
}
//*****************************************************************************
// Tasks

// Serial terminal task
void SerialEcho(void *paremeters) {
  
  char c = '\n';
  char MsgBuff [MSG_LEN]; 
  int idx = 0;
  // Clear the msgBuff
  memset(MsgBuff, 0, sizeof(MsgBuff));


  while(true) {
    if (Serial.available()>0) {
      c = Serial.read();
      // Echo back to terminal
      Serial.write(c);
      // Store received character to buffer if not over buffer limit
      if (idx < MSG_LEN - 1) {
        MsgBuff[idx] = c;
        idx++;
      }
      if (c == '\n' || c == '\r') {
        // Set nul character to the last index
        MsgBuff[idx-1] = '\0';

        // Debug print
        if (debug) {
          Serial.print("Received: ");
          Serial.println(MsgBuff);
        }

        // Compare the msgBuff to a known string
        if (strcmp(MsgBuff,command) == 0) {
          // Wait for Average value and print it.
          Serial.print("The average value is: ");
          Serial.println(AvgVal);
        }
        // Clear the msgBuff
        memset(MsgBuff, 0, sizeof(MsgBuff));
        idx = 0;
      }
    }
    // Delay a bit this task
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

// Wait for semaphore and calculate average of ADC values
void Average(void *paremeters) {
  while(true) {
    // Wait for semaphore
    xSemaphoreTake(AverageSemaphore, portMAX_DELAY);
    // Calculate average
    AvgVal = 0.0;
    for (int i = 0; i < MAX_ADC_BUFF_LEN; i++) {
      AvgVal += AdcBuff[i];
    }
    AvgVal /= MAX_ADC_BUFF_LEN;
    // Debug print
    if (debug) {
      Serial.print("Average: ");
      Serial.println(AvgVal);
    }
    // Delay a bit this task
    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}
//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Sample and Process Demo---");

  // Create semaphore
  AverageSemaphore = xSemaphoreCreateBinary();
  // Start timer to run ISR every 100ms
  timer = timerBegin(0, timer_divider, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timer_max_count, true);
  timerAlarmEnable(timer);

  // Create the tasks
  xTaskCreatePinnedToCore(SerialEcho, "SerialEcho", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(Average, "Average", 2048, NULL, 1, NULL, app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
