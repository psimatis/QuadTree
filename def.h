#ifndef QUADTREE_DEF_H
#define QUADTREE_DEF_H

// Comment accordingly to select your preferred split strategy
#define POINT_SPLIT true     // Optimized Point-Quad-Tree: splits the bucket on the median point
//#define POINT_SPLIT false    // Point-Region-Quad-Tree: divides the bucket to equal sized regions

#define CAPACITY 128

#define XLOW 0
#define YLOW 1
#define XHIGH 2
#define YHIGH 3

#define NW 0
#define NE 1
#define SW 2
#define SE 3

#endif //QUADTREE_DEF_H
