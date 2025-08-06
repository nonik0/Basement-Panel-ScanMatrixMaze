#!/usr/bin/env python3
"""
Utility to visualize the current maze data from maze.h
Run this to see what the current static maze looks like
"""

def extract_maze_from_header():
    """Extract maze data from maze.h file"""
    try:
        with open('maze.h', 'r') as f:
            content = f.read()
        
        # Find the MAZE array definition
        start_marker = 'uint16_t MAZE[] = {'
        end_marker = '};'
        
        start_idx = content.find(start_marker)
        if start_idx == -1:
            print("Could not find MAZE array definition")
            return None
            
        start_idx += len(start_marker)
        end_idx = content.find(end_marker, start_idx)
        
        if end_idx == -1:
            print("Could not find end of MAZE array")
            return None
            
        maze_data_str = content[start_idx:end_idx]
        
        # Parse binary values
        maze = []
        for line in maze_data_str.strip().split('\n'):
            line = line.strip()
            if line.startswith('0b'):
                # Remove the '0b' prefix and any trailing comma
                binary_str = line[2:].rstrip(',')
                maze.append(int(binary_str, 2))
            elif line and not line.startswith('//'):
                # Handle other formats if needed
                pass
        
        return maze
        
    except Exception as e:
        print(f"Error reading maze.h: {e}")
        return None

def visualize_maze(maze_data):
    """Visualize the maze data"""
    if not maze_data:
        print("No maze data to visualize")
        return
        
    MAZE_WIDTH = 16
    MAZE_HEIGHT = len(maze_data)
    
    print(f"Maze size: {MAZE_WIDTH} x {MAZE_HEIGHT}")
    print("# = wall, . = path, E = exit")
    print()
    
    for row_idx, row_data in enumerate(maze_data):
        line = ""
        for col in range(MAZE_WIDTH):
            bit_pos = (MAZE_WIDTH - 1) - col
            is_wall = bool(row_data & (1 << bit_pos))
            
            if is_wall:
                line += "#"
            else:
                # Check if this might be an exit (path on border)
                if (row_idx == 0 or row_idx == MAZE_HEIGHT-1 or 
                    col == 0 or col == MAZE_WIDTH-1):
                    line += "E"
                else:
                    line += "."
        print(f"{row_idx:2d}: {line}")
    
    print()
    print("Maze data as hex:")
    for i, row in enumerate(maze_data):
        print(f"Row {i}: 0x{row:04X} = 0b{row:016b}")

if __name__ == "__main__":
    print("Current maze from maze.h:")
    print("=" * 40)
    
    maze = extract_maze_from_header()
    if maze:
        visualize_maze(maze)
    else:
        print("Failed to extract maze data")