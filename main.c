#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "gifenc.h"
#include <string.h>
#include <ctype.h>

#define WIDTH 800
#define HEIGHT 600
#define WRITE_GIF 0

typedef struct parsed_rulestring {
  int born_count;
  int survive_count;
  char *rulestring;
  char *survive_string;
  char *born_string;
  int *survive_numbers;
  int *born_numbers;
} parsed_rulestring;

bool **new_grid(int width, int height);
void delete_grid(bool **grid);
void randomise_grid(bool **grid, int width, int height);

void init_grid();
void update(int width, int height, parsed_rulestring rules, ge_GIF * gif, uint16_t delay);

int count_neighbours(int x, int y, bool **grid, int width, int height);

parsed_rulestring parse_rulestring(char *rulestring);
void print_rules(parsed_rulestring rules);
bool test_live_cell(int count, int* survive_numbers, int size);
bool test_dead_cell(int count, int* born_numbers, int size);

void draw();
void draw_grid(bool **grid, int width, int height);
void draw_gif_frame(ge_GIF *gif, int width, int height, uint16_t delay);

char rulestring[] = "B3/S23";
bool paused = true;
bool **live_grid = NULL;
bool **swap_grid = NULL;
int **ghost_grid = NULL;

int **new_ghost_grid(int width, int height);
void delete_ghost_grid(int **grid);

int main(int argc, char** args) {

  parsed_rulestring rules = parse_rulestring(rulestring);
  print_rules(rules);

  init_grid();

  InitWindow(WIDTH, HEIGHT, "Game of Life");

  SetTargetFPS(30);

  uint8_t palette[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF
  };
  ge_GIF* gif = NULL;
  if(WRITE_GIF) gif = ge_new_gif("test.gif", WIDTH, HEIGHT, palette, 1, 1);
  uint16_t delay = 30 /100;

  while(!WindowShouldClose()) {
    update(WIDTH, HEIGHT, rules, gif, delay);
    draw();
  }

  delete_grid(live_grid);
  delete_grid(swap_grid);

  if(gif != NULL) {
    ge_close_gif(gif);
  }

  if(ghost_grid != NULL) {
    uint8_t ghost_palette[256 * 3];
    for(int i = 0; i < 256; i++) {
      uint8_t value = (uint8_t) i;
      int index = i * 3;
      ghost_palette[index] = value;
      ghost_palette[index+1] = value;
      ghost_palette[index+2] = value;
    }
    int depth = 8;
    ge_GIF * ghost = ge_new_gif("ghost.gif", WIDTH, HEIGHT, ghost_palette, depth, 0);


    uint8_t pixels[WIDTH * HEIGHT];
    for(int x = 0; x < WIDTH; x ++){
      for(int y = 0; y < HEIGHT; y ++) {
        int pos = (y * WIDTH) + x;
        int value = ghost_grid[x][y];
        if(value > 255) value = 255;
        if(value < 0) value = 0;
        pixels[pos] = value;
      }
    }
    memcpy(ghost->frame, pixels, sizeof(pixels));
    ge_add_frame(ghost, 0);
    ge_close_gif(ghost);
    delete_ghost_grid(ghost_grid);
  }

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
  //ghost_grid = new_ghost_grid(WIDTH, HEIGHT);
}

void update(int width, int height, parsed_rulestring rules, ge_GIF * gif, uint16_t delay) {
  
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
          new_state = test_live_cell(live_neighbours, rules.survive_numbers, rules.survive_count);
        } else {
          new_state = test_dead_cell(live_neighbours, rules.born_numbers, rules.born_count);
        }
        copy[x][y] = new_state;
        if(ghost_grid != NULL) {
          if(new_state) {
            if(ghost_grid[x][y] < 255)
              ghost_grid[x][y] += 10;
          } else {
            if(ghost_grid[x][y] > 0)
              ghost_grid[x][y] --;
          }
        }
      }
    }

    swap_grid = grid;
    live_grid = copy;

    draw_gif_frame(gif, width, height, delay);
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


parsed_rulestring parse_rulestring(char *rulestring) {
  char *working_string = malloc(strlen(rulestring));
  strcpy(working_string, rulestring);
  
  // handle B/S format
  char *divided = strtok(working_string, "/");
  if(divided == NULL) {
    fprintf(stderr, "Invalid rulestring: %s", rulestring);
    exit(1);
  }
  
  parsed_rulestring result;
  result.rulestring = rulestring;

  int token_count = 0;
  while(divided != NULL && token_count < 2) {
    char *token = divided;
    char first = token[0];

    if(first == 'B' || first == 'S') {
      int * numbers = malloc(sizeof(int) * 8);
      int number_index = 0;
      if(numbers == NULL) {
        fprintf(stderr, "Could not allocate array");
        exit(1);
      }

      int len = strlen(token);
      for(int i = 1; i < len; i ++) {
        char c = token[i];
        if(!isdigit(c)) continue;
        int number = (int) (c - '0');
        numbers[number_index] = number;
        number_index ++;
      }

      if(first == 'B') {
        result.born_string = token;
        result.born_numbers = numbers;
        result.born_count = number_index;
      } else if (first == 'S') {
        result.survive_string = token;
        result.survive_numbers = numbers;
        result.survive_count = number_index;
      }
    }

    
    token_count ++;
    divided = strtok(NULL, "/");
  }

  return result;
}

void print_rules(parsed_rulestring rules) {
  printf("Rulestring: %s\n", rules.rulestring);
  printf("Born string: %s\n", rules.born_string);
  printf("Surive string: %s\n", rules.survive_string);
}

bool test_live_cell(int count, int* survive_numbers, int size) {
  for(int i = 0; i < size; i++) {
    if(count == survive_numbers[i]) {
      return true;
    }
  }
  return false;
}
bool test_dead_cell(int count, int* born_numbers, int size) {
  for(int i = 0; i < size; i++) {
    if(count == born_numbers[i]) {
      return true;
    }
  }
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
      if(ghost_grid != NULL) {
        int val = ghost_grid[x][y];
        if(val > 255) val = 255;
        else if(val < 0) val = 0;
        Color color = (Color) {val, val, val, val};
          DrawPixel(x, y, color);
      } else {
        if(grid[x][y]) {
          DrawPixel(x, y, WHITE);
        }
      }
    }
  }
}

void draw_gif_frame(ge_GIF *gif, int width, int height, uint16_t delay) {
  if(gif == NULL) return;
  uint8_t pixels[width * height];
  for(int x = 0; x < width; x ++){
    for(int y = 0; y < height; y ++) {
      int pos = (y * width) + x;
      if(live_grid[x][y]) {
        pixels[pos] = 1;
      } else {
        pixels[pos] = 0;
      }
    }
  }
  memcpy(gif->frame, pixels, sizeof(pixels));
  ge_add_frame(gif, delay);
}

int **new_ghost_grid(int width, int height) {

  int ** grid = malloc(sizeof(int *) * width);
  if(grid == NULL) {
    fprintf(stderr, "Could not init grid");
    exit(1);
  }
  for(int x = 0; x < width; x++) {
    grid[x] = malloc(sizeof(int) * height);
    if(grid[x] == NULL) {
      fprintf(stderr, "Could not init grid");
      exit(1);
    }
    for(int y = 0; y < height; y ++) {
      grid[x][y] = 0;
    }
  }
  return grid;
}

void delete_ghost_grid(int **grid) {
  if(grid == NULL) return;
  free(grid);
  grid = NULL;
}
