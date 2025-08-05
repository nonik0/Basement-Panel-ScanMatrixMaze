#include "maze.h"
#include "scanMatrix.h" // Scan buffer for the LED matrix display

class Point
{
public:
  void set(uint16_t x, uint16_t y)
  {
    X = x;
    Y = y;
  }
  uint16_t X;
  uint16_t Y;
};

#define SCREEN_WIDTH 16
#define SCREEN_HEIGHT 16
#define SCREEN_HALF_WIDTH (SCREEN_WIDTH / 2)
#define SCREEN_HALF_HEIGHT (SCREEN_HEIGHT / 2)
#define H_INSET (SCREEN_WIDTH / 7)
#define RATIO 1.0f
#define X0 SCREEN_HALF_WIDTH
#define Y0 SCREEN_HALF_HEIGHT
#define MAX_DEPTH 3

extern uint16_t zoom, hShift;

uint16_t shift;
bool blocked;

void drawLine(int8_t x0_param, int8_t y0_param, int8_t x1, int8_t y1)
{
  // Simple line drawing - only basic math operations
  int8_t dx = x1 - x0_param;
  int8_t dy = y1 - y0_param;
  int8_t steps = (dx < 0 ? -dx : dx);
  if ((dy < 0 ? -dy : dy) > steps)
    steps = (dy < 0 ? -dy : dy);

  if (steps == 0)
  {
    if (x0_param >= 0 && x0_param < SCREEN_WIDTH && y0_param >= 0 && y0_param < SCREEN_HEIGHT)
    {
      scanSetPixel(x0_param, y0_param, true);
    }
    return;
  }

  for (int8_t i = 0; i <= steps; i++)
  {
    int8_t x = x0_param + (dx * i) / steps;
    int8_t y = y0_param + (dy * i) / steps;
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
      scanSetPixel(x, y, true);
    }
  }
}

void xToCorners(uint16_t x, Point *points)
{
  uint16_t y = (uint16_t)(RATIO * x);
  points[0].set(X0 - x, Y0 - y); // 0 = top left
  points[1].set(X0 + x, Y0 - y); // 1 = top right
  points[2].set(X0 + x, Y0 + y); // 2 = bottom right
  points[3].set(X0 - x, Y0 + y); // 3 = bottom left
}

void drawFrontLeftWall(Point *outs, Point *ins)
{
  drawLine(outs[0].X, outs[0].Y, ins[0].X, ins[0].Y);
  drawLine(ins[0].X, ins[0].Y, ins[3].X, ins[3].Y);
  drawLine(ins[3].X, ins[3].Y, outs[3].X, outs[3].Y);
}

void drawFrontRightWall(Point *outs, Point *ins)
{
  drawLine(outs[1].X, outs[1].Y, ins[1].X, ins[1].Y);
  drawLine(ins[1].X, ins[1].Y, ins[2].X, ins[2].Y);
  drawLine(ins[2].X, ins[2].Y, outs[2].X, outs[2].Y);
}

void drawBackLeftWall(Point *outs, Point *ins)
{
  drawLine(outs[0].X, ins[0].Y, ins[0].X, ins[0].Y);
  drawLine(ins[0].X, ins[0].Y, ins[3].X, ins[3].Y);
  drawLine(ins[3].X, ins[3].Y, outs[3].X, ins[3].Y);
}

void drawBackRightWall(Point *outs, Point *ins)
{
  drawLine(outs[1].X, ins[1].Y, ins[1].X, ins[1].Y);
  drawLine(ins[1].X, ins[1].Y, ins[2].X, ins[2].Y);
  drawLine(ins[2].X, ins[2].Y, outs[2].X, ins[2].Y);
}

void drawFrontWall(Point *outs, Point *ins)
{
  drawLine(outs[0].X, outs[0].Y, outs[1].X, outs[1].Y);
  drawLine(outs[1].X, outs[1].Y, outs[2].X, outs[2].Y);
  drawLine(outs[2].X, outs[2].Y, outs[3].X, outs[3].Y);
  drawLine(outs[3].X, outs[3].Y, outs[0].X, outs[0].Y);
}

void drawBackWall(Point *outs, Point *ins)
{
  drawLine(ins[0].X, ins[0].Y, ins[1].X, ins[1].Y);
  drawLine(ins[1].X, ins[1].Y, ins[2].X, ins[2].Y);
  drawLine(ins[2].X, ins[2].Y, ins[3].X, ins[3].Y);
  drawLine(ins[3].X, ins[3].Y, ins[0].X, ins[0].Y);
}

// void DebugWalls(byte depth, byte col, byte row)
//{
//   if (millis() > timeToDebug)
//   {
//     Serial.println("--------------------------");
//     Debug("depth", depth);
//     Debug("col", col);
//     Debug("row", row);
//     Debug("hasFrontLeftWall", hasFrontLeftWall);
//     Debug("hasFrontWall", hasFrontWall);
//     Debug("hasFrontRightWall", hasFrontRightWall);
//     Debug("hasBackLeftWall", hasBackLeftWall);
//     Debug("hasBackWall", hasBackWall);
//     Debug("hasBackRightWall", hasBackRightWall);
//     timeToDebug = millis() + 2000;
//   }
// }

void turnRightAnimation(Point *outs, Point *ins)
{
  for (byte i = 0; i < 4; i++)
  {
    outs[i].X = max(0, outs[i].X - hShift);
    ins[i].X = max(0, ins[i].X - hShift);
  }
}

void turnLeftAnimation(Point *outs, Point *ins)
{
  for (byte i = 0; i < 4; i++)
  {
    outs[i].X = min(SCREEN_WIDTH, outs[i].X + hShift);
    ins[i].X = min(SCREEN_WIDTH, ins[i].X + hShift);
  }
}


void drawWalls(byte depth, byte col, byte row)
{
  Point outs[4], ins[4];
  xToCorners(SCREEN_HALF_WIDTH - H_INSET * depth + (depth == 0 ? 0 : zoom), outs);
  xToCorners(SCREEN_HALF_WIDTH - H_INSET * (depth + 1) + zoom, ins);

  if (playerRotDir > 0)
    turnRightAnimation(outs, ins);
  if (playerRotDir < 0)
    turnLeftAnimation(outs, ins);

  if (playerDir == NORTH)
    lookNorth(row - depth, col);
  if (playerDir == EAST)
    lookEast(row, col + depth);
  if (playerDir == SOUTH)
    lookSouth(row + depth, col);
  if (playerDir == WEST)
    lookWest(row, col - depth);
  // DebugWalls(depth,row,col);

  if (hasFrontWall)
    drawFrontWall(outs, ins);
  else
  {
    if (hasBackWall)
      drawBackWall(outs, ins);

    if (hasFrontLeftWall)
      drawFrontLeftWall(outs, ins);
    else if (hasBackLeftWall)
      drawBackLeftWall(outs, ins);

    if (hasFrontRightWall)
      drawFrontRightWall(outs, ins);
    else if (hasBackRightWall)
      drawBackRightWall(outs, ins);
  }
}

void drawMaze()
{
  scanClear(); // Clear the scan buffer

  blocked = false; // Assume there's nothing in front of us
  for (byte depth = 0; depth < MAX_DEPTH; depth++)
    if (!blocked)
    {
      drawWalls(depth, playerCol, playerRow);
      blocked = hasFrontWall || hasBackWall;
    }

  scanShow();
}
