
#ifndef MY_LISTS_H
#define MY_LISTS_H

#include "zlist.h"

typedef struct 
{
    float x, y;
} Point;

#define REGISTER_TYPES(X) \
    X(int, Int)           \
    X(float, Float)       \
    X(Point, Point)

REGISTER_TYPES(DEFINE_LIST_TYPE)

#endif
