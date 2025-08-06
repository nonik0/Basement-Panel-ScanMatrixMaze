import random
import time

MAZE_WIDTH = 15
MAZE_HEIGHT = 9

def generate_maze():
    global start_row, start_col
    
    # Fill with walls
    maze = [0xFFFF] * MAZE_HEIGHT
    
    # Simple fixed start position to match C++ code
    start_row = 1
    start_col = 1
    
    def carve_cell(row, col):
        maze[row] &= ~(1 << ((MAZE_WIDTH - 1) - col))
    
    def in_bounds(row, col):
        return 0 < row < MAZE_HEIGHT - 1 and 0 < col < MAZE_WIDTH - 1
    
    def is_carved(row, col):
        return not (maze[row] & (1 << ((MAZE_WIDTH - 1) - col)))
    
    # Stack for recursive backtracking
    stack = []
    row, col = start_row, start_col
    carve_cell(row, col)
    stack.append((row, col))
    
    directions = [(-2, 0), (2, 0), (0, -2), (0, 2)]
    
    # Add safety counter to match C++ code
    safety_counter = 0
    max_iterations = MAZE_WIDTH * MAZE_HEIGHT * 4
    
    while stack and safety_counter < max_iterations:
        safety_counter += 1
        
        # Shuffle directions
        random.shuffle(directions)
        
        moved = False
        for dr, dc in directions:
            new_row = row + dr
            new_col = col + dc
            
            if in_bounds(new_row, new_col) and not is_carved(new_row, new_col):
                carve_cell(row + dr//2, col + dc//2)  # Carve connecting cell
                carve_cell(new_row, new_col)    # Carve destination cell
                stack.append((row, col))
                row, col = new_row, new_col
                moved = True
                break
        
        if not moved:
            row, col = stack.pop()
    
    # Create accessible exit by ensuring path connects to border
    # First, make sure there's a path at (1, MAZE_WIDTH-2)
    carve_cell(1, MAZE_WIDTH - 2)
    
    # Then create the exit on the border
    maze[1] &= ~1  # Clear rightmost bit for exit
    
    return maze

def is_wall(maze, row, col):
    if row < 0 or row >= MAZE_HEIGHT or col < 0 or col >= MAZE_WIDTH:
        return True
    return bool(maze[row] & (1 << ((MAZE_WIDTH - 1) - col)))

def print_maze(maze):
    for row in range(MAZE_HEIGHT):
        line = f"{row}: "
        for col in range(MAZE_WIDTH):
            if row == start_row and col == start_col:
                line += "S"
            elif is_wall(maze, row, col):
                line += "#"
            else:
                # Check if exit
                if ((row == 0 or row == MAZE_HEIGHT-1 or col == 0 or col == MAZE_WIDTH-1) 
                    and not is_wall(maze, row, col)):
                    line += "E"
                else:
                    line += "."
        print(line)
    print()
    
    # Also show hex representation
    print("Hex representation:")
    for i, row_data in enumerate(maze):
        print(f"Row {i}: 0x{row_data:04X}")
    print()

def main():
    random.seed(int(time.time()))
    print("Sample Generated Mazes (S = Start, # = Wall, spaces = paths)\n")
    
    for i in range(1, 6):
        print(f"Maze {i}:")
        maze = generate_maze()
        print_maze(maze)

if __name__ == "__main__":
    main()