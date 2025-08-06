
#define DEFAULT_ANIMATION_DELAY 100  // Slower animation for visibility

unsigned long autoMoveTimer = 0;
uint16_t autoMoveDelay = 1000;  // Longer pause between moves

//== Animation ==
unsigned long timeToMove;
uint16_t zoom = 0;
uint16_t zoomDir = 0;
uint16_t zoomSpeed = 1;      // Slower zoom for more frames
uint16_t hShift = 0;
uint16_t turnSpeed = 4;      // Faster turning to reduce blank space

bool canMoveInDirection(Direction dir)
{
  switch (dir) {
    case NORTH: return playerRow > 0 && !isWall(playerRow - 1, playerCol);
    case EAST:  return playerCol < (MazeColMax - 1) && !isWall(playerRow, playerCol + 1);
    case SOUTH: return playerRow < (MazeRowMax - 1) && !isWall(playerRow + 1, playerCol);
    case WEST:  return playerCol > 0 && !isWall(playerRow, playerCol - 1);
    default:    return false;
  }
}

bool move()
{
  bool needsRedraw = false;

  // Check for maze escape (only when not moving)
  if (zoomDir == 0)
  {
    if (playerRow == 0 || playerRow == (MazeRowMax - 1) || playerCol == 0 || playerCol == (MazeColMax - 1))
    {
      resetMaze();
      return true;  // Always redraw after reset
    }
  }

  // Auto-movement decision (only when idle)
  if (playerRowDir == 0 && playerColDir == 0 && playerRotDir == 0)
  {
    if (millis() > autoMoveTimer)
    {
      if (canMoveInDirection(playerDir)) {
        switch (playerDir) {
          case NORTH: playerRowDir = -1; break;
          case EAST:  playerColDir = +1; break;
          case SOUTH: playerRowDir = +1; break;
          case WEST:  playerColDir = -1; break;
        }
        zoom = 0;
        zoomDir = +zoomSpeed;
        needsRedraw = true;  // Movement started
      }
      else
      {
        playerRotDir = +1;  // Turn right when blocked
        needsRedraw = true;  // Turning started
      }
      autoMoveTimer = millis() + autoMoveDelay;
    }
  }

  // Animation updates
  if (millis() > timeToMove) {
    // Turning animation
    if (playerRotDir != 0)
    {
      hShift += turnSpeed;
      if ((hShift <= 0) || (hShift >= SCREEN_WIDTH))
      {
        hShift = 0;
        playerDir = (playerRotDir > 0) ? turnRight(playerDir) : turnLeft(playerDir);
        playerRotDir = 0;
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
        playerRow += playerRowDir;
        playerCol += playerColDir;
        playerRowDir = playerColDir = 0;
      }
      timeToMove = millis() + DEFAULT_ANIMATION_DELAY;
      needsRedraw = true;  // Animation frame update
    }
  }

  return needsRedraw;
}
