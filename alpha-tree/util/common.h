/**
 * @file common.h
 * @brief This file contains macros and variables that are used globally
 * throughout the program. It is included in every other file.
 * 
 */

#include <stdio.h>

// Globally used macros
#define BOTTOM (-1)
#define false 0
#define true 1
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#define MAX(a,b)  ((a) < (b) ? (b) : (a))

#define CONNECTIVITY 4

// Custom types needed
typedef short boolean;
typedef unsigned char ubyte;
// ubyte is an 8-bit unsigned integral data type range [0,255]
// => Pixel is an array of 3 colors in range [0,255]
typedef ubyte Pixel[3];

typedef double (*SalienceFunction)(Pixel, Pixel);
typedef double (*EdgeStrengthFunction)(Pixel *, int, int, int, int, SalienceFunction);

// constants 
extern double RGBweight[3];
extern double MainEdgeWeight;
extern double OrthogonalEdgeWeight;
extern double SalienceRange[2];

// variables
extern int width, height, size;
extern int lambda;
extern double omegafactor;

// input and output images as arrays of pixel
extern Pixel *gval;
extern Pixel *out;

// edge detection and salience functions
extern SalienceFunction salienceFunction;
extern EdgeStrengthFunction edgeStrengthX;
extern EdgeStrengthFunction edgeStrengthY;
extern EdgeStrengthFunction edgeStrengthTL_BR;
extern EdgeStrengthFunction edgeStrengthBL_TR;
// option for normalizing 
extern boolean normalize;
