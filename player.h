
#include "maze.h"

#define DEFAULT_ANIMATION_DELAY 100  // Slower animation for visibility

int playerRow, playerCol;
Direction playerHeading = NORTH;
Rotation playerRotation = NO_ROT;
Direction playerMoveDirection = NO_DIR;

unsigned long playerLastMoved = 0;
uint16_t playerMoveDelay = 1000;  // Longer pause between moves

unsigned long timeToMove;
uint16_t zoom = 0;
uint16_t zoomDir = 0;
uint16_t zoomSpeed = 1;
uint16_t hShift = 0;
uint16_t turnSpeed = 4;

bool canMoveInDirection(Direction dir)
{
  switch (dir) {
    case NORTH: return playerRow > 0 && !isWall(playerRow - 1, playerCol);
    case EAST:  return playerCol < (MAZE_WIDTH - 1) && !isWall(playerRow, playerCol + 1);
    case SOUTH: return playerRow < (MAZE_HEIGHT - 1) && !isWall(playerRow + 1, playerCol);
    case WEST:  return playerCol > 0 && !isWall(playerRow, playerCol - 1);
    case NO_DIR: return false;
    default:    return false;
  }
}

bool move()
{
  bool needsRedraw = false;

  // Check for maze escape (only when not moving)
  if (zoomDir == 0)
  {
    if (playerRow == 0 || playerRow == (MAZE_HEIGHT - 1) || playerCol == 0 || playerCol == (MAZE_WIDTH - 1))
    {
      resetMaze();
      return true;  // Always redraw after reset
    }
  }

  // Auto-movement decision (only when idle)
  if (playerMoveDirection == NO_DIR && playerRotation == NO_ROT)
  {
    if (millis() > playerLastMoved)
    {
      if (canMoveInDirection(playerHeading)) {
        playerMoveDirection = playerHeading;
        zoom = 0;
        zoomDir = +zoomSpeed;
        needsRedraw = true;  // Movement started
      }
      else
      {
        playerRotation = RIGHT;  // Turn right when blocked
        needsRedraw = true;  // Turning started
      }
      playerLastMoved = millis() + playerMoveDelay;
    }
  }

  // Animation updates
  if (millis() > timeToMove) {
    // Turning animation
    if (playerRotation != NO_ROT)
    {
      hShift += turnSpeed;
      if ((hShift <= 0) || (hShift >= SCREEN_WIDTH))
      {
        hShift = 0;
        playerHeading = (playerRotation == RIGHT) ? turnRight(playerHeading) : turnLeft(playerHeading);
        playerRotation = NO_ROT;
      }
      timeToMove = millis() + DEFAULT_ANIMATION_DELAY;
      needsRedraw = true;  // Animation frame update
    }
    // Walking animation
    else if (zoomDir != 0)
    {
      zoom += zoomDir;
      if (zoom >= H_INSET || zoom <= 0)
      {
        zoom = 0;
        zoomDir = 0;
        switch (playerMoveDirection) {
          case NORTH: playerRow--; break;
          case EAST:  playerCol++; break;
          case SOUTH: playerRow++; break;
          case WEST:  playerCol--; break;
          case NO_DIR: break;
        }
        playerMoveDirection = NO_DIR;
      }
      timeToMove = millis() + DEFAULT_ANIMATION_DELAY;
      needsRedraw = true;  // Animation frame update
    }
  }

  return needsRedraw;
}
