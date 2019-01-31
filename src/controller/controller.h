#ifndef controller_h
#define controller_h

#include <Arduino.h>
#include <BLEServer.h>

enum MODE
{
  PAIRING,
  CONNECTED
};

class Controller
{
private:
  MODE _mode = PAIRING;
  BLECharacteristic *_feedbackCharacteristic;
  BLECharacteristic *_commandCharacteristic;
  void (*_onDisconnection)(void);
  void (*_onMoveForwardCmd)(uint16_t stepNbr);
  void (*_onMoveBackwardCmd)(uint16_t stepNbr);
  void (*_onStopCmd)(void);
  void (*_onGoToZeroCmd)(void);
  void (*_onResetZeroPositionCmd)(void);

public:
  Controller();
  void notify(uint8_t *data, size_t size);
  // Command callbacks
  void setOnDisconnectionCallback(void (*onDisconnection)(void));
  void setOnMoveForwardCmdCallback(void (*onMoveForwardCmd)(uint16_t stepNbr));
  void setOnMoveBackwardCmdCallback(void (*onMoveBackwardCmd)(uint16_t stepNbr));
  void setOnStopCmdCallback(void (*onStopCmd)(void));
  void setOnGoToZeroCmdCallback(void (*onGoToZeroCmd)(void));
  void setOnResetZeroPositonCmdCallback(void (*onResetZeroPositionCmd)(void));

  class BLEServerCallbacksImpl : public BLEServerCallbacks
  {
  private:
    Controller *_controller;

  public:
    virtual void onConnect(BLEServer *pServer);
    virtual void onDisconnect(BLEServer *pServer);
    virtual void setController(Controller *controller);
  };

  class BLECharacteristicCallbacksImp : public BLECharacteristicCallbacks
  {
  private:
    Controller *_controller;

  public:
    virtual void onWrite(BLECharacteristic *pCharacteristic);
    virtual void setController(Controller *controller);
  };
};

#endif