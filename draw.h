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
extern Rotation playerRotation;
extern Direction playerHeading;
extern int playerRow, playerCol;

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

  // Clamp coordinates to screen bounds
  int16_t left = max(0, (int16_t)(X0 - x));
  int16_t right = min(SCREEN_WIDTH - 1, (int16_t)(X0 + x));
  int16_t top = max(0, (int16_t)(Y0 - y));
  int16_t bottom = min(SCREEN_HEIGHT - 1, (int16_t)(Y0 + y));
  
  points[0].set(left, top);     // 0 = top left
  points[1].set(right, top);    // 1 = top right
  points[2].set(right, bottom); // 2 = bottom right
  points[3].set(left, bottom);  // 3 = bottom left
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

void drawFrontWall(Point *outs)
{
  drawLine(outs[0].X, outs[0].Y, outs[1].X, outs[1].Y);
  drawLine(outs[1].X, outs[1].Y, outs[2].X, outs[2].Y);
  drawLine(outs[2].X, outs[2].Y, outs[3].X, outs[3].Y);
  drawLine(outs[3].X, outs[3].Y, outs[0].X, outs[0].Y);
}

void drawBackWall(Point *ins)
{
  drawLine(ins[0].X, ins[0].Y, ins[1].X, ins[1].Y);
  drawLine(ins[1].X, ins[1].Y, ins[2].X, ins[2].Y);
  drawLine(ins[2].X, ins[2].Y, ins[3].X, ins[3].Y);
  drawLine(ins[3].X, ins[3].Y, ins[0].X, ins[0].Y);
}

void drawExit(Point *ins)
{
  for (int y = ins[0].Y; y <= ins[2].Y; y++)
  {
    for (int x = ins[0].X; x <= ins[1].X; x++)
    {
      if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
      {
        scanSetPixel(x, y, true);
      }
    }
  }
}

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

  if (playerRotation == LEFT)
    turnLeftAnimation(outs, ins);
  if (playerRotation == RIGHT)
    turnRightAnimation(outs, ins);

  if (playerHeading == NORTH)
    lookNorth(row - depth, col);
  if (playerHeading == EAST)
    lookEast(row, col + depth);
  if (playerHeading == SOUTH)
    lookSouth(row + depth, col);
  if (playerHeading == WEST)
    lookWest(row, col - depth);

  if (hasExit)
  {
    drawExit(ins);
    drawFrontLeftWall(outs, ins);
    drawFrontRightWall(outs, ins);
  }
  else if (hasFrontWall)
  {
    drawFrontWall(outs);
  }
  else
  {
    if (hasBackWall)
      drawBackWall(ins);

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
  scanClear();

  for (byte depth = 0; depth < MAX_DEPTH; depth++)
  {
    drawWalls(depth, playerCol, playerRow);
    if (hasFrontWall || hasBackWall || hasExit)
      break;
  }

  scanShow();
}
