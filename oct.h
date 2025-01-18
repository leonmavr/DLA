#ifndef OCTREE_H
#define OCTREE_H

#include <stdlib.h>
#include <math.h>
#include <float.h>

#define MAX_CHILDREN 8
typedef struct {
  int x0, y0, z0, x1, y1, z1;
} cuboid_t;

typedef struct {
  int x, y, z;
  size_t id;
} point_t;

typedef struct node {
  cuboid_t boundary;
  size_t count;
  point_t *points;
  struct node *children[8];
} node_t;

typedef struct {
  node_t *root;
} octree_t;


typedef struct {
    double coordinates[3];
    int id;
} Point;

typedef struct OctreeNode {
    double center[3];
    double half_size;
    Point *point;
    struct OctreeNode *children[MAX_CHILDREN];
} OctreeNode;

typedef struct {
    OctreeNode *root;
    double boundary[3 * 2];
} Octree;

node_t *node_new(cuboid_t *boundary);
// TODO: implement this
void octree_free(Octree *tree);
void octree_insert(octree_t *octree, point_t point);
point_t octree_nearest_neighbor(octree_t *octree, point_t query);

#endif // OCTREE_H

