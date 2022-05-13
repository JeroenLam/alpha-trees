#include "EdgeDetection.h"
#include <math.h>

/**
 * @brief Computes the salience between two pixels using an unweighted average.
 * computes sqrt(sum((P1 - P2)^2))
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double SimpleSalience(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  return sqrt(result);
}

/**
 * @brief Computes the salience between two pixels using a weighted average.
 * computes sqrt(sum(W*(P1 - P2)^2))
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double WeightedSalience(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += RGBweight[i] * ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  return sqrt(result);
}

/**
 * @brief Computes the edge strength in the x direction at a given position (x,y)
 * 
 * @param img Image to compute in
 * @param width of the image
 * @param height of the image
 * @param x x-coordinate of the position
 * @param y y-coordinate of the position
 * @param Salience Pointer to a function computing the salience between two pixels
 * @return double The edge strength
 */
double EdgeStrengthX(Pixel *img, int width, int height, int x, int y, SalienceFunction Salience)
{
  int yminus1 = y - (y > 0);
  int yplus1 = y + (y < height - 1);

  // We use the minimum salience between the sourrounding rows at (x-1) and x
  double ygrad = MIN(
    Salience(
      img[width * yminus1 + x - 1],
      img[width * yplus1 + x - 1]
    ),
    Salience(
      img[width * yminus1 + x],
      img[width * yplus1 + x]
    )
  );
  return (
    OrthogonalEdgeWeight * 
    ygrad + 
    MainEdgeWeight *
    Salience(
      img[width * y + x - 1],
      img[width * y + x]
    )
  );
}

/**
 * @brief Computes the edge strength in the y direction at a given position (x,y)
 * 
 * @param img Image to compute in
 * @param width of the image
 * @param height of the image
 * @param x x-coordinate of the position
 * @param y y-coordinate of the position
 * @return double The edge strength
 */
double EdgeStrengthY(Pixel *img, int width, int height, int x, int y, SalienceFunction Salience)
{
  int xminus1 = x - (x > 0);
  int xplus1 = x + (x < width - 1);

  // We use the minimum salience between the sourrounding columns at (y-1) and y
  double xgrad = MIN(
    Salience(
      img[width * y + xplus1],
      img[width * y + xminus1]
    ),
    Salience(
      img[width * (y - 1) + xplus1],
      img[width * (y - 1) + xminus1]
    )
  );
  return (
    OrthogonalEdgeWeight * 
    xgrad + 
    MainEdgeWeight *
    Salience(
      img[width * (y - 1) + x],
      img[width * y + x]
    )
  );
}