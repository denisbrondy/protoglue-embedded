#include <Arduino.h>
#include <stepper.h>
#include "controller/controller.h"

Controller *controller;
Stepper *stepper;

void feedbackHandler(void *parameters);
void stepperHandler(void *parameters);
void moveForward(uint16_t grain);
void moveBackward(uint16_t grain);
void stopMotor();

void setup()
{
  Serial.begin(115200);
  delay(1000);
  stepper = new Stepper(GPIO_NUM_0, GPIO_NUM_15, GPIO_NUM_2, 10);
  stepper->setFrequency(20);
  controller = new Controller();
  controller->setMoveForwardCmdCallback(&moveForward);
  controller->setMoveBackwardCmdCallback(&moveBackward);
  controller->setOnDisconnectionCallback(&stopMotor);
  controller->setOnStopCmdCallback(&stopMotor);
  xTaskCreate(feedbackHandler, "FEEDBACK_THREAD", 4096 * 2, NULL, 5, NULL);
  xTaskCreate(stepperHandler, "STEPPER_THREAD", 4096 * 2, NULL, 5, NULL);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void moveForward(uint16_t grain)
{
  Serial.println("Moving forward");
  Serial.println(grain);
  stepper->move(FORWARD, (int)grain); // TBD TO CHECK THE CAST
}

void moveBackward(uint16_t grain)
{
  Serial.println("Moving backward");
  Serial.println(grain);
  stepper->move(BACKWARD, (int)grain); // TBD TO CHECK THE CAST
}

void stopMotor()
{
  Serial.println("Need to stop !!!");
  stepper->stop();
}

void feedbackHandler(void *parameters)
{
  while (true)
  {
    int32_t position = stepper->getPosition();
    uint8_t data[8] = {position, (position >> 8), (position >> 16), (position >> 24),
                       position, (position >> 8), (position >> 16), (position >> 24)};
    controller->notify(data, 8);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  vTaskDelete(NULL);
}

void stepperHandler(void *parameters)
{
  while (true)
  {
    stepper->handle();
    taskYIELD();
  }
  vTaskDelete(NULL);
}