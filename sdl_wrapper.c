#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

sdl_context_t *sdl_context_create(const char *title, int width, int height) {
  sdl_context_t *ret = malloc(sizeof(sdl_context_t));
  if (!ret) {
    fprintf(stderr, "Memory allocation failed for SDL context\n");
    return NULL;
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
    free(ret);
    return NULL;
  }

  ret->window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       width, height, SDL_WINDOW_SHOWN);
  if (!ret->window) {
    fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
    free(ret);
    SDL_Quit();
    return NULL;
  }

  ret->renderer = SDL_CreateRenderer(ret->window, -1, SDL_RENDERER_ACCELERATED);
  if (!ret->renderer) {
    fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(ret->window);
    SDL_Quit();
    return NULL;
  }

  ret->texture = SDL_CreateTexture(ret->renderer, SDL_PIXELFORMAT_RGB888,
                                   SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!ret->texture) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
    SDL_DestroyRenderer(ret->renderer);
    SDL_DestroyWindow(ret->window);
    SDL_Quit();
    return NULL;
  }

  return ret;
}

void sdl_context_release(sdl_context_t *context) {
  SDL_DestroyTexture(context->texture);
  SDL_DestroyRenderer(context->renderer);
  SDL_DestroyWindow(context->window);
  free(context);
  SDL_Quit();
}

bool sdl_context_render(sdl_context_t *context, uint32_t **array, int width,
                        int height) {
  SDL_Event e;
  bool is_done = false;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      is_done = true;
      return is_done;
    }
  }

  // Copy the modified array into the texture
  void *pixels;
  int pitch;
  SDL_LockTexture(context->texture, NULL, &pixels, &pitch);

  for (int r = 0; r < height; ++r) {
    uint32_t *row = (uint32_t *)((uint8_t *)pixels + r * pitch);
    for (int c = 0; c < width; ++c) {
      row[c] = array[r][c];
    }
  }

  SDL_UnlockTexture(context->texture);

  // Render the texture
  SDL_RenderClear(context->renderer);
  SDL_RenderCopy(context->renderer, context->texture, NULL, NULL);
  SDL_RenderPresent(context->renderer);
  return is_done;
}
