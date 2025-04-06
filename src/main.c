
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

  Apple apple;
  apple.x = (rand() % ((WIDTH - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
  apple.y = (rand() % ((HEIGHT - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;

  Uint32 lastMove = SDL_GetTicks();
  const Uint32 moveDelay = 1;

  int current_state, next_state, action, reward;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = 0;
    }

    Uint32 now = SDL_GetTicks();
    if (now - lastMove >= moveDelay) {
      lastMove = now;

      current_state = get_state(snake[0].x, snake[0].y, apple.x, apple.y, dx, dy);
      action = choose_action(current_state);

      int temp_dx = dx, temp_dy = dy;
      if (action == 0) { dx = -temp_dy; dy = temp_dx; }
      else if (action == 2) { dx = temp_dy; dy = -temp_dx; }

      for (int i = snake_length - 1; i > 0; i--) snake[i] = snake[i - 1];
      snake[0].x += dx * BLOCK_SIZE;
      snake[0].y += dy * BLOCK_SIZE;

      reward = -1;


      if (hit_and_clip(snake, snake_length)) {
        int score = (snake_length - 1) * 10; 
        if (score > high) high = score;

        printf("[Episode %d] Length: %d | Score: %d | High Score: %d\n",
              episodes, snake_length, score, high);
        fflush(stdout);

        reward = -100;
        snake_length = 1;
        snake[0].x = WIDTH / 2;
        snake[0].y = HEIGHT / 2;
        dx = 1; dy = 0;
        episodes++;
      }


      if (snake[0].x == apple.x && snake[0].y == apple.y) {
        reward = 10;
        if (snake_length < MAX_SNAKE_LENGTH) snake_length++;
        apple.x = (rand() % ((WIDTH - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;
        apple.y = (rand() % ((HEIGHT - 2 * BLOCK_SIZE) / BLOCK_SIZE)) * BLOCK_SIZE + BLOCK_SIZE;

      }

      next_state = get_state(snake[0].x, snake[0].y, apple.x, apple.y, dx, dy);
      update_q(current_state, action, reward, next_state);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
      glVertex2i(apple.x, apple.y);
      glVertex2i(apple.x + BLOCK_SIZE, apple.y);
      glVertex2i(apple.x + BLOCK_SIZE, apple.y + BLOCK_SIZE);
      glVertex2i(apple.x, apple.y + BLOCK_SIZE);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < snake_length; i++) {
      glBegin(GL_QUADS);
        glVertex2i(snake[i].x, snake[i].y);
        glVertex2i(snake[i].x + BLOCK_SIZE, snake[i].y);
        glVertex2i(snake[i].x + BLOCK_SIZE, snake[i].y + BLOCK_SIZE);
        glVertex2i(snake[i].x, snake[i].y + BLOCK_SIZE);
      glEnd();
    }

    // Walls
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

