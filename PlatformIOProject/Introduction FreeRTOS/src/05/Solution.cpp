/**
 * FreeRTOS Queue Demo
 * 
  * One task performs basic echo on Serial. If it sees "delay" followed by a
 * number, it sends the number (in a queue) to the second task. If it receives
 * a message in a second queue, it prints it to the console. The second task
 * blinks an LED. When it gets a message from the first queue (number), it
 * updates the blink delay to that number. Whenever the LED blinks 100 times,
 * the second task sends a message to the first task to be printed.
 * 
 * Date: January 19, 2020
 * Author: Raúl Sainz
 * License: 0BSD
 */

#include <Arduino.h>
// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define LED_PIN 13
#define MSG_LEN 1000

// Settings
static const uint8_t msg_queue_len = 10;
static const uint8_t delay_queue_len = 10;

// Globals
static QueueHandle_t msg_queue;
static QueueHandle_t delay_queue;
static const int led_pin = LED_PIN;


// Typedefs
typedef struct {
  int n;              // Numerber of blinked times 
  char msg[MSG_LEN];  // Message to be printed
} msg_t;

//*****************************************************************************
// Tasks

// Serial handler task: echo the characters writen on serial, if "delay xxx" is received, it sends the number xxx to the delay queue
void SerialTask(void *parameters) {

  static int delay = 100;
  static msg_t msg;
  static msg_t msg_BlinkTask;
  static int i = 0;
  static char c;
  // Loop forever
  while (1) {
    // Echo characters
    if (Serial.available() > 0) {
      c = Serial.read();
      msg.msg[i] = c;
      i++;
      Serial.write(c);
    }
    // If the message is complete
    if (c == '\n' && i != 0) {
      // If the message is "delay xxx"
      if (strncmp(msg.msg, "delay", 5) == 0) {
        // Get the number from the message
        delay = atoi(msg.msg + 6);
        delay <= 10 ? delay = 10 : delay = delay;
        // Send the number to the delay queue
        if (xQueueSend(delay_queue, &delay, 0) != pdPASS) {
          Serial.println("Error sending to delay queue");
        }
      } else{
        Serial.println("No delay found. Please write in this form: delay XXX");
      }
      // Clear msg buffer
      memset(msg.msg, 0, MSG_LEN);
      i = 0;
    }

    // Print messages from Task Blink LED:
    if (xQueueReceive(msg_queue, &msg_BlinkTask,0) != pdPASS) {
      // Que empty, do nothing
    }else{
      Serial.print(msg_BlinkTask.msg);
      if (msg_BlinkTask.n > 0) {
        Serial.print(msg_BlinkTask.n);
        Serial.println(" times");
      }else{
        Serial.println(msg.msg); 
      } 
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

// Blink task: blinks the LED every time it receives a message from the delay queue
void BlinkTask (void *parameters) {
  
  static int delay = 500;
  static msg_t msg;
  msg.n = 0;
  int cnt = 0;

  // Loop forever
  while (1) {
    if (xQueueReceive(delay_queue, &delay,0) != pdPASS) {

    }else{
      String text = "Delay updated to:" + String(delay) + "ms";
      strcpy(msg.msg,text.c_str());
      msg.n = 0;
      if (xQueueSend(msg_queue, &msg,0) != pdPASS) {
        Serial.println("Error sending message to Task1");
      }
    }
    // Report how many times LED blinked
    cnt++;
    if (cnt%100 == 0) {
      msg.n = cnt;
      strcpy(msg.msg ,"Blinked: ");
      if (xQueueSend(msg_queue, &msg,0) != pdPASS) {
        Serial.println("Error sending message to Task1");
      }
    }
    // Blink LED
    digitalWrite(led_pin, LOW);
    vTaskDelay(delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, HIGH);
    vTaskDelay(delay / portTICK_PERIOD_MS);
  }
}
//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure Serial
  Serial.begin(115200);
  
  // set led pin as output
  pinMode(led_pin, OUTPUT);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS 05 Queue Solution---");

  // Create queues
  msg_queue = xQueueCreate(msg_queue_len, sizeof(msg_t));
  delay_queue = xQueueCreate(delay_queue_len, sizeof(int));

  // Start tasks
  xTaskCreatePinnedToCore(SerialTask,
                          "Print Messages",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  xTaskCreatePinnedToCore(BlinkTask,
                          "BLink LED",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  // Delte setup and loop task
  vTaskDelete(NULL);
}

void loop() {

}
