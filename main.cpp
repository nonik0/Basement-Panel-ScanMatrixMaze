
#include <Arduino.h>

#include "draw.h"
#include "maze.h"
#include "player.h"
#include "scanMatrix.h"

void setup(void)
{
  scanInit();
  scanDisplay(true);
}

void loop(void)
{
  if (move()) {
    drawMaze();
  }
}
