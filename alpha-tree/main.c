#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <assert.h>

#include "util/common.h"
#include "util/PPMImageReadWrite.h"
#include "util/EdgeDetection.h"
#include "util/DistanceMeasures.h"
#include "util/TreeFilter.h"
#include "source/EdgeQueue.h"
#include "source/SalienceTree.h"

double RGBweight[3] = {0.5, 0.5, 0.5};
double MainEdgeWeight = 1.0;
double OrthogonalEdgeWeight = 1.0;
double SalienceRange[2] = {0, 10000};

// variables
int width, height, size;
int lambda;
int *depths;
double omegafactor = 200000;

// input and output images as arrays of pixel
Pixel *gval = NULL;
Pixel *out = NULL;

// Set the function you want to use to compute the alpha between two pixels here
SalienceFunction salienceFunction = &ManhattenDistance;
// Set the functions you want to use to compute edge strength here
EdgeStrengthFunction edgeStrengthX = &EdgeStrengthX;
EdgeStrengthFunction edgeStrengthY = &EdgeStrengthY;
// diagonals for 8-Connectivity
EdgeStrengthFunction edgeStrengthTL_BR = NULL;
EdgeStrengthFunction edgeStrengthBL_TR = NULL;
boolean normalize = true;

int main(int argc, char *argv[])
{

  char *imgfname, *outfname = "out.ppm";
  int r;
  unsigned long i;
  clock_t start;
  struct tms tstruct;
  long tickspersec = sysconf(_SC_CLK_TCK);
  float musec;
  SalienceTree *tree;

  // Check if the right amount of arguments are provided and set variables according to them
  if (argc < 3)
  {
    printf("Usage: %s <input image> <lambda>  [omegafactor] [output image] \n", argv[0]);
    exit(0);
  }

  imgfname = argv[1];

  lambda = atoi(argv[2]);
  if (argc > 3)
    omegafactor = atof(argv[3]);

  if (argc > 4)
    outfname = argv[4];

  // Read the input image
  // This sets both the global gval pixel array (input image)
  // as well as the dimensions of the image (height, width, size)
  if (!ImagePPMRead(imgfname))
    return (-1);

  // allocate space for the pixel array that is the output image
  out = malloc(size * sizeof(Pixel));

  printf("Filtering image '%s' using attribute area with lambda=%d\n", imgfname, lambda);
  printf("Image: Width=%d Height=%d\n", width, height);

  printf("Data read, start filtering.\n");
  start = times(&tstruct);
  // create the actual alpha tree
  tree = MakeSalienceTree(gval, width, height, (double)lambda);

  musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

  printf("wall-clock time: %f s\n", musec);
  // apply what we have found in the alpha tree creation to the out image
  // here colors and areas are created etc.
  SalienceTreeAreaFilter(tree,out,lambda);
  // SalienceTreeSalienceFilter(tree, out, (double)lambda);
  // SalienceTreeColorMapFilter(tree, out, (double)lambda);

  musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

  printf("wall-clock time: %f s\n", musec);

  r = ImagePPMBinWrite(outfname);
  free(out);
  if (r)
    printf("Filtered image written to '%s'\n", outfname);

  free(gval);
  free(depths);
  return (0);
} /* main */
