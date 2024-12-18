#include "camera.h"
#include <stdio.h>

int main() {
  printf("start\n");
  scene_init(0, 0, 300, 70, 70);
  sphere_t spheres[3];
  spheres[0] = sphere_make(0, 25, 600, 100, 0, 0, 200);
  spheres[1] = sphere_make(100, 50, 700, 100, 200, 0, 0);
  spheres[2] = sphere_make(-50, 100, 2000, 550, 0, 200, 50);
  for (int i = 0; i < sizeof(spheres) / sizeof(spheres[0]); ++i) {
    sphere_write(&spheres[i]);
  }
  pbuffer_save("output.ppm");
  buffer_free();
  return 0;
}
