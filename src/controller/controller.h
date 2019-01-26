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

  public:
    Controller();
    void start();

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