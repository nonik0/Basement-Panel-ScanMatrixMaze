#pragma once

#include <Arduino.h>

enum Direction {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3
};

#define NUM_DIRECTIONS 4


// Direction manipulation functions
inline Direction turnLeft(Direction dir) {
  return (Direction)((dir + 3) % NUM_DIRECTIONS); // Same as (dir - 1 + 4) % 4
}

inline Direction turnRight(Direction dir) {
  return (Direction)((dir + 1) % NUM_DIRECTIONS);
}

inline Direction getOppositeDirection(Direction dir) {
  return (Direction)((dir + 2) % NUM_DIRECTIONS);
}


// uint16_t MAZE[] = {
//   0b1111111111110111,
//   0b1100000011110111,
//   0b1111101110000001,
//   0b1111101111010111,
//   0b1000001111000001,
//   0b1101101111110111,
//   0b1000000000000111,
//   0b1111111111111111,
// };
uint16_t MAZE[] = {
  0b1011111111111111,
  0b1011111111111111,
  0b1011111111111111,
  0b1011111111111111,
  0b1011111111111111,
  0b1011111111111111,
  0b1111111111111111,
  0b1111111111111111,
};

#define MAZE_WIDTH 16
#define MAZE_HEIGHT 8
byte MazeColMax = MAZE_WIDTH;
byte MazeRowMax = MAZE_HEIGHT;

int playerRow, playerCol; // Where you are in the Maze
Direction playerDir = NORTH; // Facing direction: NORTH, EAST, SOUTH, WEST
int playerRotDir = 0; // Turning -1=Left, 0=none, 1=Right
int playerRowDir = 0, playerColDir = 0; // Delta when walking

bool hasFrontLeftWall;
bool hasFrontWall;
bool hasFrontRightWall;
bool hasBackLeftWall;
bool hasBackWall;
bool hasBackRightWall;
bool hasExit;

void resetMaze()
{
  // playerCol = 12;
  // playerRow = 3;
  playerCol = 1;
  playerRow = 5;
  playerDir = NORTH;
}

bool isWall(byte row, byte col)
{
  if (row < 0 || row > MazeRowMax || col < 0 || col > MazeColMax)
    return false;
  else
  {
    return bitRead(MAZE[row], (MAZE_WIDTH - 1) - col);
  }
}

bool isExitPosition(byte row, byte col)
{
  // Check if this position is at the maze boundary (exit)
  return (row <= 0 || row >= (MazeRowMax - 1) || col <= 0 || col >= (MazeColMax - 1));
}

void lookNorth(byte row, byte col)
{
  hasFrontLeftWall = isWall(row, col - 1);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row, col + 1);
  hasBackLeftWall  = isWall(row - 1, col - 1);
  hasBackWall      = isWall(row - 1, col);
  hasBackRightWall = isWall(row - 1, col + 1);
  hasExit          = isExitPosition(row, col);
}

void lookEast(byte row, byte col)
{
  hasFrontLeftWall = isWall(row - 1, col);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row + 1, col);
  hasBackLeftWall  = isWall(row - 1, col + 1);
  hasBackWall      = isWall(row,  col + 1);
  hasBackRightWall = isWall(row + 1, col + 1);
  hasExit          = isExitPosition(row, col);
}

void lookSouth(byte row, byte col)
{
  hasFrontLeftWall = isWall(row, col + 1);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row, col - 1);
  hasBackLeftWall  = isWall(row + 1, col + 1);
  hasBackWall      = isWall(row + 1, col);
  hasBackRightWall = isWall(row + 1, col - 1);
  hasExit          = isExitPosition(row + 1, col);
}

void lookWest(byte row, byte col)
{
  hasFrontLeftWall = isWall(row + 1, col);
  hasFrontWall     = isWall(row,     col);
  hasFrontRightWall = isWall(row - 1, col);
  hasBackLeftWall  = isWall(row + 1, col - 1);
  hasBackWall      = isWall(row,     col - 1);
  hasBackRightWall = isWall(row - 1, col - 1);
  hasExit          = isExitPosition(row, col - 1);
}

