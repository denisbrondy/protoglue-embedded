#ifndef controller_h
#define controller_h

#include <Arduino.h>
#include <BLEServer.h>

enum MODE
{
  PAIRING,
  CONNECTED
};

enum SPEED
{
  LOW_SPEED,
  MEDIUM_SPEED,
  HIGH_SPEED
};

enum COMMAND
{
  NONE,
  FORWARDCMD,
  BACKWARDCMD,
  STOPCMD,
  GOTOZEROCMD,
  RESETZEROPOSITIONCMD,
  SETSPEEDCMD
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
  void (*_onSetSpeedCmd)(SPEED speed);

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
  void setOnSetSpeedCmdCallback(void (*onSetSpeedCmd)(SPEED speed));

  static String getCmdName(COMMAND command)
  {
    String commandString = "";
    switch (command)
    {
    case NONE:
      
      break;
    case FORWARDCMD:
      commandString.concat("FORWARD");
      break;
    case BACKWARDCMD:
      commandString.concat("BACKWARD");
      break;
    case STOPCMD:
      commandString.concat("STOP");
      break;
    case GOTOZEROCMD:
      commandString.concat("GO TO ZERO");
      break;
    case RESETZEROPOSITIONCMD:
      commandString.concat("RESET ZERO POSITION");
      break;
    case SETSPEEDCMD:
      commandString.concat("SET SPEED");
      break;
    default:
      commandString.concat("UNKNOWN");
      break;
    }
    return commandString;
  }

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