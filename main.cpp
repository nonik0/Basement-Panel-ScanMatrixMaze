
#include <Arduino.h>
#include <Wire.h>

#include "draw.h"
#include "maze.h"
#include "player.h"
#include "scan.h"

#define I2C_ADDRESS 0x15

volatile bool display = true;

void handleOnReceive(int bytesReceived)
{
  uint8_t command = Wire.read();

  // setDisplay
  if (command == 0x00)
  {
    display = Wire.read();
    scanDisplay(display);
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
  if (display && move()) {
    drawMaze();
  }
}
