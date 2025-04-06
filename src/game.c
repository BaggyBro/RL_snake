
#include "../include/game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>

void handle_input(int *dx, int *dy) {
  const Uint8 *keys = SDL_GetKeyboardState(NULL);

  if (keys[SDL_SCANCODE_W] && *dy != 1) {
    *dx = 0;
    *dy = -1;
  } else if (keys[SDL_SCANCODE_S] && *dy != -1) {
    *dx = 0;
    *dy = 1;
  } else if (keys[SDL_SCANCODE_A] && *dx != 1) {
    *dx = -1;
    *dy = 0;
  } else if (keys[SDL_SCANCODE_D] && *dx != -1) {
    *dx = 1;
    *dy = 0;
  }
}

