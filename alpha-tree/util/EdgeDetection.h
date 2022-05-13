#ifndef EDGE_DETECTION_H
#define EDGE_DETECTION_H

#include "common.h"

double simpleSalience(Pixel p, Pixel q);
double WeightedSalience(Pixel p, Pixel q);
double EdgeStrengthX(Pixel *img, int width, int height, int x, int y);
double EdgeStrengthY(Pixel *img, int width, int height, int x, int y);

#endif