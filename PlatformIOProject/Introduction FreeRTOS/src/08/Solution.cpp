/**
 * FreeRTOS Software Timer Demo
 * 
 * Turn on LED when entering serial commands. Turn it off if serial is inactive
 * for 5 seconds.
 * 
 * Date: February 1, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */

// You'll likely need this on vanilla FreeRTOS
//#include <timers.h>
#include <Arduino.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define EXTERNAL_LED 13
#define SERIAL_TIMEOUT_MS 5000
// Globals
static TimerHandle_t SerialTimer = NULL;
static const int external_led = EXTERNAL_LED;
// Task

void SerialEcho(void *paremeters) {
  
  char c = '\n';
  TickType_t xRemainingTime;
  int RemainingTime_s;
  int RemainingTime_s_old;
  while(true) {
    if (Serial.available()>0) {
      c = Serial.read();
      Serial.write(c);
      // Start timers (max block time if command queue is full)
      xTimerStart(SerialTimer, portMAX_DELAY);
      digitalWrite(external_led,HIGH);
    } else {
      xRemainingTime = xTimerGetExpiryTime( SerialTimer ) - xTaskGetTickCount();
      RemainingTime_s = (int) xRemainingTime/1000;
      if (RemainingTime_s > 0 && RemainingTime_s <= (SERIAL_TIMEOUT_MS/1000) && RemainingTime_s_old != RemainingTime_s) {
        Serial.print( "Timer will expire in: ");
        Serial.print (RemainingTime_s);
        Serial.println(" s");
      }
      RemainingTime_s_old = RemainingTime_s;
      vTaskDelay(100/portTICK_PERIOD_MS);
    }
  }
}

//*****************************************************************************
// Callbacks

// Called when one of the timers expires
void myTimerCallback(TimerHandle_t xTimer) {
  digitalWrite(external_led,LOW);
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);
  // Configure led
  pinMode(external_led,OUTPUT);
  digitalWrite(external_led,LOW);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Timer Solution---");

  // Create a one-shot timer
  SerialTimer = xTimerCreate(
                  "Serial Timer",           // Name of timer
                  SERIAL_TIMEOUT_MS / portTICK_PERIOD_MS,  // Period of timer (in ticks)
                  pdFALSE,                    // Auto-reload
                  (void *)0,                  // Timer ID
                  myTimerCallback);           // Callback function

  // Create task to exho characters
  xTaskCreatePinnedToCore(
                        SerialEcho,
                        "Serial Echo",
                        1024,
                        NULL,
                        1,
                        NULL,
                        app_cpu);

  // Check to make sure timers were created
  if (SerialTimer == NULL) {
    Serial.println("Could not create timer");
  }
  // Delete self task to show that timers will work with no user tasks
  vTaskDelete(NULL);
}


void loop() {
  // Execution should never get here
}
