#include "util.h"
#include <math.h>

/**
 * @brief Computes the dot product between two vectors represented by Pixels
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double The dot product between them
 */
double DotProduct(Pixel p, Pixel q)
{
  double dot_product = 0;
  for (int i = 0; i < 3; i++)
  {
    dot_product += (double)p[i] * (double)q[i];
  }
  return dot_product;
}

/**
 * @brief Computes the magnitude of a vector represented by a Pixel
 * 
 * @param p Pixel to get the magnitude of
 * @return double Magnitude of the Pixel
 */
double Magnitude(Pixel p)
{
  double mag = 0;
  for (int i = 0; i < 3; i++)
  {
    mag += (double)p[i] * (double)p[i];
  }
  return sqrt(mag);
}

/**
 * @brief Maps a given value from one range of values to another range of values
 * 
 * @param x Value to map
 * @param lower_in Lower bound of the original range
 * @param upper_in Upper bound of the original range
 * @param lower_out Lower bound of the new range
 * @param upper_out Upper bound of the new range
 * @return double Newly mapped value
 */
double Map(double x, double lower_in, double upper_in, double lower_out, double upper_out)
{
  return lower_out + ((upper_out - lower_out) / (upper_in - lower_in)) * (x - lower_in);
}

/**
 * @brief Prints the values of a given Pixel to stdout
 * 
 * @param p Pixel to investigate
 */
void printPixel(Pixel p)
{
  printf("{%d,%d,%d}", p[0], p[1], p[2]);
}
