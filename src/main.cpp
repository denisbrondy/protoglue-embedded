#include <Arduino.h>
#include <stepper.h>
#include "controller/controller.h"

// STEPPER MOTOR IS 1.8° PER STEP SO 200 STEPS FOR A COMPLETE ROTATION
uint16_t LOW_FREQUENCY = 20;     // => 1/10 rotation per second
uint16_t MEDIUM_FREQUENCY = 200; // => 1 complete rotation per second
uint16_t HIGH_FREQUENCY = 2000;   // => 10 complete rotations per second

Controller *controller;
Stepper *stepper;

void feedbackHandler(void *parameters);
void stepperHandler(void *parameters);
void moveForward(uint16_t stepNbr);
void moveBackward(uint16_t stepNbr);
void stopMotor();
void goToZero();
void resetZeroPosition();
void setMotorSpeed(SPEED speed);

void setup()
{
  Serial.begin(115200);
  delay(1000);
  stepper = new Stepper(GPIO_NUM_0, GPIO_NUM_15, GPIO_NUM_4, LOW_FREQUENCY);
  controller = new Controller();
  controller->setOnMoveForwardCmdCallback(&moveForward);
  controller->setOnMoveBackwardCmdCallback(&moveBackward);
  controller->setOnDisconnectionCallback(&stopMotor);
  controller->setOnStopCmdCallback(&stopMotor);
  controller->setOnGoToZeroCmdCallback(&goToZero);
  controller->setOnResetZeroPositonCmdCallback(&resetZeroPosition);
  controller->setOnSetSpeedCmdCallback(&setMotorSpeed);
  xTaskCreate(feedbackHandler, "FEEDBACK_THREAD", 4096 * 2, NULL, 5, NULL);
  xTaskCreate(stepperHandler, "STEPPER_THREAD", 4096 * 2, NULL, 5, NULL);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void moveForward(uint16_t stepNbr)
{
  Serial.println("Moving forward");
  Serial.println(stepNbr);
  stepper->move(FORWARD, (int)stepNbr); // TBD TO CHECK THE CAST
}

void moveBackward(uint16_t stepNbr)
{
  Serial.println("Moving backward");
  Serial.println(stepNbr);
  stepper->move(BACKWARD, (int)stepNbr); // TBD TO CHECK THE CAST
}

void stopMotor()
{
  Serial.println("Need to stop !!!");
  stepper->stop();
}

void goToZero()
{
  Serial.println("Going to zero position");
  stepper->goToZero();
}

void resetZeroPosition()
{
  Serial.println("Resetting zero position");
  stepper->resetZeroPosition();
}

void setMotorSpeed(SPEED speed)
{
  Serial.println("Setting speed");
  Serial.println(speed);
  switch (speed)
  {
  case LOW_SPEED:
    stepper->setFrequency(LOW_FREQUENCY);
    break;
  case MEDIUM_SPEED:
    stepper->setFrequency(MEDIUM_FREQUENCY);
    break;
  case HIGH_SPEED:
    stepper->setFrequency(HIGH_FREQUENCY);
    break;
  default:
    stepper->setFrequency(LOW_FREQUENCY);
    break;
  }
}

void feedbackHandler(void *parameters)
{
  while (true)
  {
    int32_t position = stepper->getPosition();
    uint8_t moving = (uint8_t)stepper->moving();
    uint8_t data[5] = {position, (position >> 8), (position >> 16), (position >> 24),
                       moving};
    controller->notify(data, 5);
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