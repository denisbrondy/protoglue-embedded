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
        uint16_t stepNbr = (data[1] << 8) + data[2];
        this->_controller->_moveForwardCmd(stepNbr);
    }
    else if (command == 2)
    {
        uint16_t stepNbr = (data[1] << 8) + data[2];
        this->_controller->_moveBackwardCmd(stepNbr);
    }
    else if (command == 3)
    {
        this->_controller->_onStopCmd();
    }
    else if (command == 4)
    {
        this->_controller->_onGoToZeroCmd();
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
}

void Controller::setOnDisconnectionCallback(void (*onDisconnection)(void))
{
    this->_onDisconnection = onDisconnection;
}

void Controller::setMoveForwardCmdCallback(void (*moveForwardCmd)(uint16_t stepNbr))
{
    this->_moveForwardCmd = moveForwardCmd;
}

void Controller::setMoveBackwardCmdCallback(void (*moveBackwardCmd)(uint16_t stepNbr))
{
    this->_moveBackwardCmd = moveBackwardCmd;
}

void Controller::setOnStopCmdCallback(void (*onStopCmd)(void))
{
    this->_onStopCmd = onStopCmd;
}

void Controller::setOnGoToZeroCallback(void (*onGoToZeroCmd)(void))
{
    this->_onGoToZeroCmd = onGoToZeroCmd;
}

void Controller::notify(uint8_t *data, size_t size)
{
    if (this->_mode == CONNECTED)
    {
        this->_feedbackCharacteristic->setValue(data, size);
        this->_feedbackCharacteristic->notify();
    }
}