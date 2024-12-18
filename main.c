#include "camera.h"
#include "utils.h"
#include <stdio.h>

int main() {
  printf("start\n");
  scene_init(0, 0, 300, 70, 70);
  for (int i = 0; i < 300 + xrandom() % 200; ++i) {
    sphere_t sphere = sphere_make(
        -600 + xrandom() % 1200, -400 + xrandom() % 800, 800, 30,
        150 + xrandom() % 50, 150 + xrandom() % 50, 150 + xrandom() % 50);
    sphere_write(&sphere);
  }
  pbuffer_save("output.ppm");
  buffer_free();
  return 0;
}
