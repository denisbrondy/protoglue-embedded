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
BLECharacteristic *feedbackCharacteristic; // Feedbacks
BLECharacteristic *commandCharacteristic;  // Command

void Controller::BLEServerCallbacksImpl::setController(Controller *controller)
{
    this->_controller = controller;
}

void Controller::BLEServerCallbacksImpl::onConnect(BLEServer *pServer)
{
    Serial.println("Bluetooth connected to pair");
    _controller->_mode = CONNECTED;
};

void Controller::BLEServerCallbacksImpl::onDisconnect(BLEServer *pServer)
{
    Serial.println("Bluetooth disconnected with pair");
    this->_controller->_onDisconnection();
    _controller->_mode = PAIRING;
};

void Controller::BLECharacteristicCallbacksImp::onWrite(BLECharacteristic *pCharacteristic)
{
    char const *data = pCharacteristic->getValue().c_str();
    uint8_t command = data[0] & 0xFF;
    Serial.println("Received command : ");
    Serial.println(command);
    if (command == 1)
    {
        uint16_t grain = (data[1] << 8) + data[2];
        this->_controller->_moveForwardCmd(grain);
    }
    else if (command == 2)
    {
        uint16_t grain = (data[1] << 8) + data[2];
        this->_controller->_moveBackwardCmd(grain);
    }
    else if (command == 3)
    {
    }
    else if (command == 4)
    {
    }
    else if (command == 5)
    {
        this->_controller->_onStopCmd();
    }
    else if (command == 6)
    {
        Serial.println("MAKE COURSE");
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
    feedbackCharacteristic = pService->createCharacteristic(
        FEEDBACK_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
    feedbackCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristicCallbacksImp *lBLECharacteristicCallbacksImp = new Controller::BLECharacteristicCallbacksImp();
    lBLECharacteristicCallbacksImp->setController(this);

    // Command
    commandCharacteristic = pService->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    commandCharacteristic->setCallbacks(lBLECharacteristicCallbacksImp);
    commandCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("Controller started");
}

void Controller::setOnDisconnectionCallback(void (*onDisconnection)(void))
{
    this->_onDisconnection = onDisconnection;
}

void Controller::setMoveForwardCmdCallback(void (*moveForwardCmd)(uint16_t grain))
{
    this->_moveForwardCmd = moveForwardCmd;
}

void Controller::setMoveBackwardCmdCallback(void (*moveBackwardCmd)(uint16_t grain))
{
    this->_moveBackwardCmd = moveBackwardCmd;
}

void Controller::setOnStopCmdCallback(void (*onStopCmd)(void)) {
    this->_onStopCmd = onStopCmd;
}

void Controller::notify(uint8_t *data, size_t size)
{
    if (this->_mode == CONNECTED)
    {
        feedbackCharacteristic->setValue(data, size);
        feedbackCharacteristic->notify();
    }
}