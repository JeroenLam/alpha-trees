#include "DistanceMeasures.h"
#include "util.h"
#include <math.h>

/**
 * @brief Computes the salience between two pixels using an unweighted average.
 * computes sqrt(sum((P1 - P2)^2))
 * 
 * Returns value in the range [0, sqrt(3*(255^2))]
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double EuclideanDistance(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  result = sqrt(result);
  if (normalize)
    result = Map(result, 0.0, sqrt(3*255*255), SalienceRange[0], SalienceRange[1]);
  return result;
}

/**
 * @brief Computes the salience between two pixels using a weighted average.
 * computes sqrt(sum(W*(P1 - P2)^2))
 * 
 * Returns value in the range [0, sqrt(3*(255^2))]
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double WeightedEuclideanDistance(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += RGBweight[i] * ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  result = sqrt(result);
  if (normalize)
    result = Map(result, 0.0, sqrt(3.0*255.0*255.0), SalienceRange[0], SalienceRange[1]);
  return result;
}

/**
 * @brief Computes the salience between two pixels using manhatten distance.
 * computes sum(abs(P1-P2))
 * 
 * Returns value in the range [0, 765]
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double ManhattenDistance(Pixel p, Pixel q)
{
  double result = 0;
  for (int i = 0; i < 3; i++)
  {
    result += (double)fabs((double)p[i] - (double)q[i]);
  }
  if (normalize)
    result = Map(result, 0.0, 765.0, SalienceRange[0], SalienceRange[1]);
  return result;
}

/**
 * @brief Computes the salience between two pixels using their cosine similarity.
 * computes 1 - abs((dot(P1,P2))/(|P1|*|P2|))
 * 
 * Returns value in the range [0, 1]
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double CosineDistance(Pixel p, Pixel q)
{
  double dot_product = DotProduct(p, q);
  double mag_p = Magnitude(p);
  double mag_q = Magnitude(q);
  double cosine = fabs(dot_product / (mag_p * mag_q));
  double result = 1.0 - cosine;
  if (normalize)
    result = Map(result, 0.0, 1.0, SalienceRange[0], SalienceRange[1]);
  return result;
}
