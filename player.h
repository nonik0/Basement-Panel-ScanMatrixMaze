
#include "maze.h"

#define DEFAULT_ANIMATION_DELAY 100  // Slower animation for visibility

int playerRow, playerCol;
Direction playerHeading = NORTH;
Rotation playerRotation = NO_ROT;
Direction playerMoveDirection = NO_DIR;
bool justTurned = false;

unsigned long playerLastMoved = 0;
uint16_t playerMoveDelay = 500;  // Longer pause between moves

unsigned long timeToMove;
uint16_t zoom = 0;
uint16_t zoomDir = 0;
uint16_t zoomSpeed = 1;
uint16_t hShift = 0;
uint16_t turnSpeed = 4;

bool atExit() {
  return (playerRow == 0 || playerRow == (MAZE_HEIGHT - 1) || playerCol == 0 || playerCol == (MAZE_WIDTH - 1));
}

bool canMoveInDirection(Direction dir)
{
  switch (dir) {
    case NORTH:
      return (playerRow > 0 && !isWall(playerRow - 1, playerCol)) || (playerRow == 0 && atExit());
    case EAST:
      return (playerCol < (MAZE_WIDTH - 1) && !isWall(playerRow, playerCol + 1)) || (playerCol == (MAZE_WIDTH - 1) && atExit());
    case SOUTH:
      return (playerRow < (MAZE_HEIGHT - 1) && !isWall(playerRow + 1, playerCol)) || (playerRow == (MAZE_HEIGHT - 1) && atExit());
    case WEST:
      return (playerCol > 0 && !isWall(playerRow, playerCol - 1)) || (playerCol == 0 && atExit());
    default:
      return false;
  }
}

bool move()
{
  bool needsRedraw = false;

  // Auto-movement decision using modified right-hand rule (only when idle)
  if (playerMoveDirection == NO_DIR && playerRotation == NO_ROT)
  {
    if (millis() > playerLastMoved)
    {
      // navigate through maze always going right at turns when possible, then straight forward, then left at corners 
      if (!justTurned && canMoveInDirection(turnRight(playerHeading))) {
        playerRotation = RIGHT;
        justTurned = true;
        needsRedraw = true;
      }
      else if (canMoveInDirection(playerHeading)) {
        playerMoveDirection = playerHeading;
        zoom = 0;
        zoomDir = +zoomSpeed;
        justTurned = false;
        needsRedraw = true;
      }
      else if (canMoveInDirection(turnLeft(playerHeading))) {
        playerRotation = LEFT;
        justTurned = true;
        needsRedraw = true;
      }
      else {
        playerRotation = RIGHT;
        justTurned = true;
        needsRedraw = true;
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
      needsRedraw = true;
    }
    // Walking animation
    else if (zoomDir != 0)
    {
      zoom += zoomDir;

      if (atExit())
      {
        if (zoom >= H_INSET * 2) {
          zoom = 0;
          zoomDir = 0;
          playerMoveDirection = NO_DIR;

          delay(1000);
          resetMaze();
          return true;  // Reset maze if at exit
        }
      }
      else if (zoom >= H_INSET)
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
      needsRedraw = true;
    }
  }

  return needsRedraw;
}
