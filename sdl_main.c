#include "sdl_wrapper.h"

static unsigned frame_offset = 0;
int main() {
  int width = 640;
  int height = 480;

  // Allocate memory for the 2D array
  uint32_t **array = malloc(height * sizeof(uint32_t *));
  if (!array) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }
  for (int i = 0; i < height; i++) {
    array[i] = malloc(width * sizeof(uint32_t));
    if (!array[i]) {
      fprintf(stderr, "Memory allocation failed\n");
      for (int j = 0; j < i; j++) {
        free(array[j]);
      }
      free(array);
      return 1;
    }
  }

  // Initialize the array with initial gradient values
  for (int r = 0; r < height; r++) {
    for (int c = 0; c < width; c++) {
      uint8_t red = (uint8_t)((c * 255) / width);    // Horizontal gradient
      uint8_t green = (uint8_t)((r * 255) / height); // Vertical gradient
      uint8_t blue = (uint8_t)(255 - ((r + c) * 255) / (width + height));
      array[r][c] = (red << 16) | (green << 8) | blue;
    }
  }

  // Draw the array using SDL
  sdl_context_t *context =
      sdl_context_create("Iterative Rendering", width, height);
  if (!context) {
    return 1;
  }

  bool is_done = false;
  while (!is_done) {
    // Modify array values dynamically for iterative rendering
    for (int r = 0; r < height; r++) {
      for (int c = 0; c < width; c++) {
        uint8_t red = (uint8_t)(((c * 255) / width) + frame_offset) % 256;
        uint8_t green = (uint8_t)(((r * 255) / height) + frame_offset) % 256;
        uint8_t blue = (uint8_t)(255 - (((r + c) * 255) / (width + height) +
                                        frame_offset) %
                                           256);
        array[r][c] = (red << 16) | (green << 8) | blue;
      }
    }

    // Increment the frame offset for dynamic color changes
    is_done = sdl_context_render(context, array, width, height);
    frame_offset++;
    SDL_Delay(16); // Delay to cap frame rate to ~60 FPS
  }

  // Cleanup
  sdl_context_release(context);
  // Free the array memory
  for (int i = 0; i < height; i++) {
    free(array[i]);
  }
  free(array);

  return 0;
}
