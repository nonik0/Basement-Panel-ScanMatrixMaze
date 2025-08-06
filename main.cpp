
#include <Arduino.h>
#include <Wire.h>

#include "draw.h"
#include "maze.h"
#include "player.h"
#include "scan.h"

#define I2C_ADDRESS 0x15
#define STATUS_LED_PIN 5
#define STATUS_UPDATE_INTERVAL 500

// display state
volatile bool display = true;

// i2c
unsigned long lastStatusLedUpdate = 0;
unsigned long statusLedUpdateInterval = STATUS_UPDATE_INTERVAL;
bool statusLedState = false;
bool statusLedFirstBlink = false;
uint8_t statusLedBlinks = 0; // number of extra short blinks after long "ACK" blink

void handleOnReceive(int bytesReceived)
{
  statusLedState = true;
  digitalWrite(STATUS_LED_PIN, !statusLedState);
  lastStatusLedUpdate = millis();
  statusLedBlinks = 0;
  statusLedFirstBlink = true;

  if (bytesReceived < 2)
  {
    return;
  }

  uint8_t command = Wire.read();
  statusLedBlinks = command + 1; // use value to blink status LED

  // setDisplay
  if (command == 0x00)
  {
    display = Wire.read();
    scanDisplay(display);
  }
  else
  {
    statusLedBlinks = 10;
  }
}

void updateStatusLed() {
  // status LED indicates when I2C message is received, long blink first, then short blink count indicates status
  if ((statusLedBlinks > 0 || statusLedState) && millis() - lastStatusLedUpdate > statusLedUpdateInterval)
  {
    statusLedState = !statusLedState;
    digitalWrite(STATUS_LED_PIN, !statusLedState);
    lastStatusLedUpdate = millis();

    if (statusLedBlinks > 0)
    {
      statusLedBlinks -= statusLedState ? 1 : 0;

      // longer gap between first long blink and later short blinks
      if (statusLedFirstBlink)
      {
        statusLedFirstBlink = false;
        statusLedUpdateInterval = STATUS_UPDATE_INTERVAL >> 1;
      }
      else
      {
        statusLedUpdateInterval = statusLedState ? (STATUS_UPDATE_INTERVAL >> 2) : (STATUS_UPDATE_INTERVAL >> 3);
      }
    }
    else
    {
      statusLedUpdateInterval = STATUS_UPDATE_INTERVAL;
    }
  }
}

void setup(void)
{
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(handleOnReceive);

  scanInit();
  scanDisplay(true);

  resetMaze();
}

void loop(void)
{
  updateStatusLed();

  if (display && move())
  {
    drawMaze();
  }
}
