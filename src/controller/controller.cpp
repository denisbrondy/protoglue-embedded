#include "controller.h"
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stepper.h>

// BLUETOOTH DEFINITIONS
const std::string DEVICE = "PROTOGLUE";
#define SERVICE_UUID "39ead0db-0bbf-449a-9af9-24001ea09aa3"
#define FEEDBACK_CHARACTERISTIC_UUID "38149ecc-adb2-4ff3-89d5-6083d52c5e9f"
#define COMMAND_CHARACTERISTIC_UUID "8a21ad2c-279d-41d1-94bf-6916ecbb3695"

void Controller::BLEServerCallbacksImpl::setController(Controller *controller)
{
    this->_controller = controller;
}

void Controller::BLEServerCallbacksImpl::onConnect(BLEServer *pServer)
{
    Serial.println("Bluetooth connected to pair");
    _controller->_mode = CONNECTED;
    digitalWrite(GPIO_NUM_2, LOW);
};

void Controller::BLEServerCallbacksImpl::onDisconnect(BLEServer *pServer)
{
    Serial.println("Bluetooth disconnected with pair");
    this->_controller->_onDisconnection();
    _controller->_mode = PAIRING;
    digitalWrite(GPIO_NUM_2, HIGH);
};

void Controller::BLECharacteristicCallbacksImp::onWrite(BLECharacteristic *pCharacteristic)
{
    char const *data = pCharacteristic->getValue().c_str();
    uint8_t command = data[0] & 0xFF;
    // Serial.println("Received command : ");
    // Serial.println(Controller::getCmdName((COMMAND)command));
    if (command == FORWARDCMD)
    {
        uint16_t stepNbr = (data[1] << 8) + data[2];
        this->_controller->_onMoveForwardCmd(stepNbr);
    }
    else if (command == BACKWARDCMD)
    {
        uint16_t stepNbr = (data[1] << 8) + data[2];
        this->_controller->_onMoveBackwardCmd(stepNbr);
    }
    else if (command == STOPCMD)
    {
        this->_controller->_onStopCmd();
    }
    else if (command == GOTOZEROCMD)
    {
        this->_controller->_onGoToZeroCmd();
    }
    else if (command == RESETZEROPOSITIONCMD)
    {
        this->_controller->_onResetZeroPositionCmd();
    }
    else if (command == SETSPEEDCMD)
    {
        uint8_t speed = data[1] & 0xFF;
        this->_controller->_onSetSpeedCmd((SPEED)speed);
    }
};

void Controller::BLECharacteristicCallbacksImp::setController(Controller *controller)
{
    this->_controller = controller;
}

Controller::Controller()
{
    WiFi.mode(WIFI_MODE_NULL);
    BLEDevice::init(DEVICE);
    BLEServer *pServer = BLEDevice::createServer();
    BLEServerCallbacksImpl *lBLEServerCallbacksImpl = new Controller::BLEServerCallbacksImpl();
    lBLEServerCallbacksImpl->setController(this);
    pServer->setCallbacks(lBLEServerCallbacksImpl);

    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Feedback
    this->_feedbackCharacteristic = pService->createCharacteristic(
        FEEDBACK_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
    this->_feedbackCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristicCallbacksImp *lBLECharacteristicCallbacksImp = new Controller::BLECharacteristicCallbacksImp();
    lBLECharacteristicCallbacksImp->setController(this);

    // Command
    this->_commandCharacteristic = pService->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    this->_commandCharacteristic->setCallbacks(lBLECharacteristicCallbacksImp);
    this->_commandCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("Controller started");

    pinMode(GPIO_NUM_2, OUTPUT);
    digitalWrite(GPIO_NUM_2, HIGH);
}

void Controller::setOnDisconnectionCallback(void (*onDisconnection)(void))
{
    this->_onDisconnection = onDisconnection;
}

void Controller::setOnMoveForwardCmdCallback(void (*onMoveForwardCmd)(uint16_t stepNbr))
{
    this->_onMoveForwardCmd = onMoveForwardCmd;
}

void Controller::setOnMoveBackwardCmdCallback(void (*onMoveBackwardCmd)(uint16_t stepNbr))
{
    this->_onMoveBackwardCmd = onMoveBackwardCmd;
}

void Controller::setOnStopCmdCallback(void (*onStopCmd)(void))
{
    this->_onStopCmd = onStopCmd;
}

void Controller::setOnGoToZeroCmdCallback(void (*onGoToZeroCmd)(void))
{
    this->_onGoToZeroCmd = onGoToZeroCmd;
}

void Controller::setOnResetZeroPositonCmdCallback(void (*onResetZeroPositionCmd)(void))
{
    this->_onResetZeroPositionCmd = onResetZeroPositionCmd;
}

void Controller::setOnSetSpeedCmdCallback(void (*onSetSpeedCmd)(SPEED speed))
{
    this->_onSetSpeedCmd = onSetSpeedCmd;
}

void Controller::notify(uint8_t *data, size_t size)
{
    if (this->_mode == CONNECTED)
    {
        this->_feedbackCharacteristic->setValue(data, size);
        this->_feedbackCharacteristic->notify();
    }
}