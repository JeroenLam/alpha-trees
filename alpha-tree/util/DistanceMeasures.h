#ifndef DISTANCE_MEASURES_H
#define DISTANCE_MEASURES_H

#include "common.h"

double EuclideanDistance(Pixel p, Pixel q);
double WeightedEuclideanDistance(Pixel p, Pixel q);
double ManhattenDistance(Pixel p, Pixel q);
double CosineDistance(Pixel p, Pixel q);

#endif