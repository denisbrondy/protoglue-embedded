#include <Arduino.h>
#include "controller/controller.h"



void setup()
{
  Serial.begin(115200);
  delay(1000);
  Controller controller;
  controller.start();
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}