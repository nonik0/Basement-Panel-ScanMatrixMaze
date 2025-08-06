#pragma once

#include <Arduino.h>

enum Direction {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3,
  NO_DIR = 4
};

enum Rotation {
  LEFT = -1,
  NO_ROT = 0,
  RIGHT = 1
};

#define MAZE_WIDTH 15
#define MAZE_HEIGHT 9
#define NUM_DIRECTIONS 4

inline Direction turnLeft(Direction dir) {
  return (Direction)((dir + 3) % NUM_DIRECTIONS); // Same as (dir - 1 + 4) % 4
}

inline Direction turnRight(Direction dir) {
  return (Direction)((dir + 1) % NUM_DIRECTIONS);
}

inline Direction getOppositeDirection(Direction dir) {
  return (Direction)((dir + 2) % NUM_DIRECTIONS);
}


uint16_t MAZE[MAZE_HEIGHT] = {
  0b1111111111110111,
  0b1000000011110111,
  0b1111101110000001,
  0b1111101111010111,
  0b1000001111000001,
  0b1111101111110111,
  0b1000000000000111,
  0b1111111111111111,
  0b1111111111111111,
};
// uint16_t MAZE[] = {
//   0b1011111111111111,
//   0b1011111111111111,
//   0b1011111111111111,
//   0b1011111111111111,
//   0b1011111111111111,
//   0b1000001111111111,
//   0b1111111111111111,
//   0b1111111111111111,
// };

// Forward declarations for player variables
extern int playerRow, playerCol;
extern Direction playerHeading;

// Maze generation variables
int startRow, startCol;

bool hasFrontLeftWall;
bool hasFrontWall;
bool hasFrontRightWall;
bool hasBackLeftWall;
bool hasBackWall;
bool hasBackRightWall;
bool hasExit;

struct Stack {
  byte data[MAZE_WIDTH * MAZE_HEIGHT][2];
  int top;
  
  void push(byte r, byte c) { data[top][0] = r; data[top][1] = c; top++; }
  void pop(byte& r, byte& c) { top--; r = data[top][0]; c = data[top][1]; }
  bool empty() { return top == 0; }
};

void carveCell(byte row, byte col) {
  MAZE[row] &= ~(1 << ((MAZE_WIDTH - 1) - col));
}

bool inBounds(byte row, byte col) {
  return row > 0 && row < MAZE_HEIGHT - 1 && col > 0 && col < MAZE_WIDTH - 1;
}

bool isCarved(byte row, byte col) {
  return !(MAZE[row] & (1 << ((MAZE_WIDTH - 1) - col)));
}

void generateMaze() {
  randomSeed(millis());
  
  // Fill with walls
  for (int i = 0; i < MAZE_HEIGHT; i++) MAZE[i] = 0xFFFF;
  
  // Simple fixed start position to avoid random issues
  startRow = 1;
  startCol = 1;
  
  Stack stack = {{}, 0};
  byte row = startRow, col = startCol;
  carveCell(row, col);
  stack.push(row, col);
  
  int directions[][2] = {{-2,0}, {2,0}, {0,-2}, {0,2}};
  
  // Add safety counter to prevent infinite loops
  int safetyCounter = 0;
  const int maxIterations = MAZE_WIDTH * MAZE_HEIGHT * 4;
  
  while (!stack.empty() && safetyCounter < maxIterations) {
    safetyCounter++;
    
    // Shuffle directions
    for (int i = 3; i > 0; i--) {
      int j = random(i + 1);
      int temp0 = directions[i][0], temp1 = directions[i][1];
      directions[i][0] = directions[j][0]; directions[i][1] = directions[j][1];
      directions[j][0] = temp0; directions[j][1] = temp1;
    }
    
    bool moved = false;
    for (int i = 0; i < 4; i++) {
      byte newRow = row + directions[i][0];
      byte newCol = col + directions[i][1];
      
      if (inBounds(newRow, newCol) && !isCarved(newRow, newCol)) {
        carveCell(row + directions[i][0]/2, col + directions[i][1]/2);
        carveCell(newRow, newCol);
        stack.push(row, col);
        row = newRow;
        col = newCol;
        moved = true;
        break;
      }
    }
    
    if (!moved) stack.pop(row, col);
  }
  
  // Create accessible exit by ensuring path connects to border
  // First, make sure there's a path at (1, MAZE_WIDTH-2) 
  carveCell(1, MAZE_WIDTH - 2);
  
  // Then create the exit on the border
  MAZE[1] &= ~1;  // Clear rightmost bit for exit
}

void resetMaze()
{
  generateMaze();
  playerCol = startCol;
  playerRow = startRow;
  playerHeading = EAST;
}

bool isWall(byte row, byte col)
{
  if (row < 0 || row >= MAZE_HEIGHT || col < 0 || col >= MAZE_WIDTH)
    return false;
  else
  {
    return bitRead(MAZE[row], (MAZE_WIDTH - 1) - col);
  }
}

bool isExitPosition(byte row, byte col)
{
  // Check if this position is at the maze boundary (exit)
  return (row <= 0 || row >= (MAZE_HEIGHT - 1) || col <= 0 || col >= (MAZE_WIDTH - 1));
}

void lookNorth(byte row, byte col)
{
  hasFrontLeftWall = isWall(row, col - 1);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row, col + 1);
  hasBackLeftWall  = isWall(row - 1, col - 1);
  hasBackWall      = isWall(row - 1, col);
  hasBackRightWall = isWall(row - 1, col + 1);
  hasExit          = !hasFrontWall && isExitPosition(row, col);
}

void lookEast(byte row, byte col)
{
  hasFrontLeftWall = isWall(row - 1, col);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row + 1, col);
  hasBackLeftWall  = isWall(row - 1, col + 1);
  hasBackWall      = isWall(row,  col + 1);
  hasBackRightWall = isWall(row + 1, col + 1);
  hasExit          = !hasFrontWall && isExitPosition(row, col);
}

void lookSouth(byte row, byte col)
{
  hasFrontLeftWall = isWall(row, col + 1);
  hasFrontWall     = isWall(row, col);
  hasFrontRightWall = isWall(row, col - 1);
  hasBackLeftWall  = isWall(row + 1, col + 1);
  hasBackWall      = isWall(row + 1, col);
  hasBackRightWall = isWall(row + 1, col - 1);
  hasExit          = !hasFrontWall && isExitPosition(row, col);
}

void lookWest(byte row, byte col)
{
  hasFrontLeftWall = isWall(row + 1, col);
  hasFrontWall     = isWall(row,     col);
  hasFrontRightWall = isWall(row - 1, col);
  hasBackLeftWall  = isWall(row + 1, col - 1);
  hasBackWall      = isWall(row,     col - 1);
  hasBackRightWall = isWall(row - 1, col - 1);
  hasExit          = !hasFrontWall && isExitPosition(row, col);
}
