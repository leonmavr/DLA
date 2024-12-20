#include "sdl_wrapper.h"
#include "utils.h"
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

static inline int lerp_int(int a, int b, float t) {
  return (int)(a + t * (b - a));
}

static inline int lmap_int(int x, int a, int b, int c, int d) {
  float t = (float)(x - a) / (b - a);
  return lerp_int(c, d, t);
}

#if 0
/** Linear interpolation (lerp) between two scalars */
static inline float lerp_float(float a, float b, float t) {
  return floor(a + t * (b - a));
}

static inline float lmap_float(float x, float a, float b, float c, float d) {
  float t = (x - a) / (b - a);
  return lerp_float(c, d, t);
}
#endif

bool sdl_context_render(sdl_context_t *context, uint32_t **array, int width,
                        int height) {
  SDL_Event e;
  bool is_done = false;

  // Handle events
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      is_done = true;
      return is_done;
    }
  }

  // Lock texture to access its pixel buffer
  void *pixels;
  int pitch;
  if (SDL_LockTexture(context->texture, NULL, &pixels, &pitch) != 0) {
    printf("SDL_LockTexture failed: %s\n", SDL_GetError());
    return is_done;
  }

  // Get window dimensions
  int wwidth = 0, wheight = 0;
  SDL_GetWindowSize(context->window, &wwidth, &wheight);

  // Iterate over texture dimensions
  for (int r = 0; r < wheight; ++r) {
    uint32_t *row = (uint32_t *)((uint8_t *)pixels + r * pitch);

    // Map each row to the source array
    int source_r = lmap_int(r, 0, wheight - 1, 0, height - 1);

    for (int c = 0; c < wwidth; ++c) {
      int source_c = lmap_int(c, 0, wwidth - 1, 0, width - 1);

      // Ensure mapped indices are within bounds
      if (source_r >= 0 && source_r < height && source_c >= 0 &&
          source_c < width) {
        row[c] = array[source_r][source_c];
      } else {
        row[c] = 0; // Default to black if out of bounds
      }
    }
  }

  SDL_UnlockTexture(context->texture);

  // Render the texture
  SDL_RenderClear(context->renderer);
  SDL_RenderCopy(context->renderer, context->texture, NULL, NULL);
  SDL_RenderPresent(context->renderer);

  return is_done;
}
