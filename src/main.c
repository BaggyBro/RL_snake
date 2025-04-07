#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/game.h"
#include "../include/rl.h"

#define WIDTH 800
#define HEIGHT 800
#define BLOCK_SIZE 20
#define MAX_SNAKE_LENGTH 1000

typedef struct {
  int x, y;
} Apple;

typedef struct {
  int x, y;
} Segment;

int hit_and_clip(Segment *snake, int length) {
  for (int i = 1; i < length; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) return 1;
  }
  if (snake[0].x < BLOCK_SIZE || snake[0].x >= WIDTH - BLOCK_SIZE ||
      snake[0].y < BLOCK_SIZE || snake[0].y >= HEIGHT - BLOCK_SIZE) return 1;
  return 0;
}

// Function to check if a position is occupied by the snake
int is_position_occupied(Segment *snake, int length, int x, int y) {
  for (int i = 0; i < length; i++) {
    if (snake[i].x == x && snake[i].y == y) return 1;
  }
  return 0;
}

// Check if a position is a wall
int is_wall(int x, int y) {
  return (x < BLOCK_SIZE || x >= WIDTH - BLOCK_SIZE ||
          y < BLOCK_SIZE || y >= HEIGHT - BLOCK_SIZE);
}

int main() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("Snakey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  srand(time(NULL));
  int running = 1;
  SDL_Event event;

  Segment snake[MAX_SNAKE_LENGTH];
  int snake_length = 1;
  snake[0].x = WIDTH / 2;
  snake[0].y = HEIGHT / 2;
  int dx = 1, dy = 0;

  int high = 0;
  int episodes = 0;
  int total_steps = 0;
  int steps_this_episode = 0;

  Apple apple;
  apple.x = (rand() % ((WIDTH - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
  apple.y = (rand() % ((HEIGHT - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;

  Uint32 lastMove = SDL_GetTicks();
  const Uint32 moveDelay = 1; // Very fast for training

  int current_state, next_state, action, reward;
  
  // Initialize Q-table
  initialize_q_table();

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = 0;
    }

    Uint32 now = SDL_GetTicks();
    if (now - lastMove >= moveDelay) {
      lastMove = now;
      total_steps++;
      steps_this_episode++;

      // Calculate dangers for state representation
      int ahead_x = snake[0].x + dx * BLOCK_SIZE;
      int ahead_y = snake[0].y + dy * BLOCK_SIZE;
      int danger_ahead = is_wall(ahead_x, ahead_y) || is_position_occupied(snake, snake_length, ahead_x, ahead_y);
      
      int left_dx = -dy, left_dy = dx;
      int left_x = snake[0].x + left_dx * BLOCK_SIZE;
      int left_y = snake[0].y + left_dy * BLOCK_SIZE;
      int danger_left = is_wall(left_x, left_y) || is_position_occupied(snake, snake_length, left_x, left_y);
      
      int right_dx = dy, right_dy = -dx;
      int right_x = snake[0].x + right_dx * BLOCK_SIZE;
      int right_y = snake[0].y + right_dy * BLOCK_SIZE;
      int danger_right = is_wall(right_x, right_y) || is_position_occupied(snake, snake_length, right_x, right_y);

      current_state = get_state(snake[0].x, snake[0].y, apple.x, apple.y, dx, dy, 
                               danger_ahead, danger_left, danger_right);
      action = choose_action(current_state, episodes);

      int prev_x = snake[0].x;
      int prev_y = snake[0].y;

      int temp_dx = dx, temp_dy = dy;
      if (action == 0) { 
          // Turn left
          dx = -temp_dy; 
          dy = temp_dx; 
      } 
      else if (action == 1) {
      } 
      else if (action == 2) { 
          dx = temp_dy; 
          dy = -temp_dx; 
      }

      for (int i = snake_length - 1; i > 0; i--) snake[i] = snake[i - 1];
      snake[0].x += dx * BLOCK_SIZE;
      snake[0].y += dy * BLOCK_SIZE;

      reward = -0.1;  

      int prev_dist = abs(prev_x - apple.x) + abs(prev_y - apple.y);
      int curr_dist = abs(snake[0].x - apple.x) + abs(snake[0].y - apple.y);
      
      if (curr_dist < prev_dist) {
          reward += 1.0;
      } else if (curr_dist > prev_dist) {
          reward -= 0.5;
      }

      if (hit_and_clip(snake, snake_length)) {
        int score = (snake_length - 1) * 10; 
        if (score > high) high = score;

        printf("[Episode %d] Length: %d | Score: %d | High Score: %d | Steps: %d\n",
              episodes, snake_length, score, high, steps_this_episode);
        fflush(stdout);

        reward = -100;  // Big penalty for dying
        snake_length = 1;
        snake[0].x = WIDTH / 2;
        snake[0].y = HEIGHT / 2;
        dx = 1; dy = 0;
        episodes++;
        steps_this_episode = 0;
        
        decay_epsilon();
      }

      if (snake[0].x == apple.x && snake[0].y == apple.y) {
        reward = 20;  
        if (snake_length < MAX_SNAKE_LENGTH) snake_length++;
        
        int valid_position = 0;
        while (!valid_position) {
            apple.x = (rand() % ((WIDTH - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
            apple.y = (rand() % ((HEIGHT - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
            valid_position = !is_position_occupied(snake, snake_length, apple.x, apple.y);
        }
      }

      ahead_x = snake[0].x + dx * BLOCK_SIZE;
      ahead_y = snake[0].y + dy * BLOCK_SIZE;
      danger_ahead = is_wall(ahead_x, ahead_y) || is_position_occupied(snake, snake_length, ahead_x, ahead_y);
      
      left_dx = -dy; left_dy = dx;
      left_x = snake[0].x + left_dx * BLOCK_SIZE;
      left_y = snake[0].y + left_dy * BLOCK_SIZE;
      danger_left = is_wall(left_x, left_y) || is_position_occupied(snake, snake_length, left_x, left_y);
      
      right_dx = dy; right_dy = -dx;
      right_x = snake[0].x + right_dx * BLOCK_SIZE;
      right_y = snake[0].y + right_dy * BLOCK_SIZE;
      danger_right = is_wall(right_x, right_y) || is_position_occupied(snake, snake_length, right_x, right_y);

      next_state = get_state(snake[0].x, snake[0].y, apple.x, apple.y, dx, dy,
                            danger_ahead, danger_left, danger_right);
      update_q(current_state, action, reward, next_state);

      if (episodes % 100 == 0 && steps_this_episode == 1) {
          save_q_table(episodes);
      }
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // Draw apple
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
      glVertex2i(apple.x, apple.y);
      glVertex2i(apple.x + BLOCK_SIZE, apple.y);
      glVertex2i(apple.x + BLOCK_SIZE, apple.y + BLOCK_SIZE);
      glVertex2i(apple.x, apple.y + BLOCK_SIZE);
    glEnd();

    // Draw snake
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < snake_length; i++) {
      glBegin(GL_QUADS);
        glVertex2i(snake[i].x, snake[i].y);
        glVertex2i(snake[i].x + BLOCK_SIZE, snake[i].y);
        glVertex2i(snake[i].x + BLOCK_SIZE, snake[i].y + BLOCK_SIZE);
        glVertex2i(snake[i].x, snake[i].y + BLOCK_SIZE);
      glEnd();
    }

    // Draw walls
    glBegin(GL_QUADS);
      glVertex2i(0, 0); glVertex2i(WIDTH, 0); glVertex2i(WIDTH, BLOCK_SIZE); glVertex2i(0, BLOCK_SIZE);
      glVertex2i(0, HEIGHT - BLOCK_SIZE); glVertex2i(WIDTH, HEIGHT - BLOCK_SIZE); glVertex2i(WIDTH, HEIGHT); glVertex2i(0, HEIGHT);
      glVertex2i(0, 0); glVertex2i(BLOCK_SIZE, 0); glVertex2i(BLOCK_SIZE, HEIGHT); glVertex2i(0, HEIGHT);
      glVertex2i(WIDTH - BLOCK_SIZE, 0); glVertex2i(WIDTH, 0); glVertex2i(WIDTH, HEIGHT); glVertex2i(WIDTH - BLOCK_SIZE, HEIGHT);
    glEnd();

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
