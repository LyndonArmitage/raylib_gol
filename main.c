#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

bool **new_grid(int width, int height);
void delete_grid(bool **grid);
void randomise_grid(bool **grid, int width, int height);

void init_grid();
void update(int width, int height);

int count_neighbours(int x, int y, bool **grid, int width, int height);
bool test_live_cell(int count);
bool test_dead_cell(int count);

void draw();
void draw_grid(bool **grid, int width, int height);

bool paused = false;
bool **live_grid = NULL;
bool **swap_grid = NULL;

int main(int argc, char** args) {

  init_grid();

  InitWindow(WIDTH, HEIGHT, "Game of Life");

  SetTargetFPS(30);

  while(!WindowShouldClose()) {
    update(WIDTH, HEIGHT);
    draw();
  }

  delete_grid(live_grid);
  delete_grid(swap_grid);

  return 0;
}

void init_grid() {
  if(live_grid != NULL) {
    delete_grid(live_grid);
  }
  if(swap_grid != NULL) {
    delete_grid(swap_grid);
  }
  live_grid = new_grid(WIDTH, HEIGHT);
  swap_grid = new_grid(WIDTH, HEIGHT);
}

void update(int width, int height) {
  
  if(IsKeyPressed(KEY_R)) {
    randomise_grid(live_grid, width, height);
  }
  if(IsKeyPressed(KEY_P)) {
    paused = !paused;
  }
  if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    live_grid[GetMouseX()][GetMouseY()] = true;
  }

  if(!paused) {
    bool **grid = live_grid;
    bool **copy = swap_grid;

    for(int x = 0; x < width; x ++) {
      for(int y = 0; y < height; y ++) {
        bool new_state = false;
        int live_neighbours = count_neighbours(x, y, grid, width, height);
        if(grid[x][y]) {
          new_state = test_live_cell(live_neighbours);
        } else {
          new_state = test_dead_cell(live_neighbours);
        }
        copy[x][y] = new_state;
      }
    }

    swap_grid = grid;
    live_grid = copy;
  }
}

int count_neighbours(int x, int y, bool **grid, int width, int height) {
  if(grid == NULL) return 0;
  int count = 0;

  int north_y = y < height-1 ? y+1 : 0;
  int south_y = y > 0 ? y-1 : height-1;
  int east_x = x < width-1 ? x+1 : 0;
  int west_x = x > 0 ? x-1 : width-1;


  // north
  if(grid[x][north_y]) count ++;
  // northeast
  if(grid[east_x][north_y]) count ++;
  // east
  if(grid[east_x][y]) count ++;
  // southeast
  if(grid[east_x][south_y]) count ++;
  // south
  if(grid[x][south_y]) count ++;
  // southwest
  if(grid[west_x][south_y]) count ++;
  // west
  if(grid[west_x][y]) count ++;
  // northwest
  if(grid[west_x][north_y]) count ++; 

  return count;
}

bool test_live_cell(int count) {
  if(count < 2)
    return false;
  else if(count >= 2 && count <= 3)
    return true;
  else
    return false;
}

bool test_dead_cell(int count) {
  if(count == 3)
    return true;
  else
    return false;
}

bool **new_grid(int width, int height) {
  bool ** grid = malloc(sizeof(bool *) * width);
  if(grid == NULL) {
    fprintf(stderr, "Could not init grid");
    exit(1);
  }
  for(int x = 0; x < width; x++) {
    grid[x] = malloc(sizeof(bool) * height);
    if(grid[x] == NULL) {
      fprintf(stderr, "Could not init grid");
      exit(1);
    }
    for(int y = 0; y < height; y ++) {
      grid[x][y] = false;
    }
  }
  return grid;
}

void delete_grid(bool **grid) {
  if(grid == NULL) return;
  free(grid);
  grid = NULL;
}

void randomise_grid(bool **grid, int width, int height) {
  if(grid == NULL) return;
  for(int x = 0; x < width; x ++) {
    for(int y = 0; y < height; y ++) {
      grid[x][y] = GetRandomValue(0, 1);
    }
  }
}

void draw() {
  BeginDrawing();
  ClearBackground(BLACK);
  draw_grid(live_grid, WIDTH, HEIGHT);
  EndDrawing();
}

void draw_grid(bool **grid, int width, int height) {
  if(grid == NULL) return;
  for(int x = 0; x < width; x ++) {
    for(int y = 0; y < height; y ++) {
      if(grid[x][y]) {
        DrawPixel(x, y, WHITE);
      }
    }
  }
}
