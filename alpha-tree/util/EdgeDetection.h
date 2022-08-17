#ifndef EDGE_DETECTION_H
#define EDGE_DETECTION_H

#include <math.h>

#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Vec;
using cv::Mat;

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
template<typename chType, unsigned int nCh>
double EdgeStrengthX(Mat img, Point p, SalienceFunction Salience)
{
  int yminus1 = p.y - (p.y > 0);
  int yplus1 = p.y + (p.y < img.rows - 1);

  // We use the minimum salience between the sourrounding rows at (x-1) and x
  double ygrad = MIN(
    Salience(
      img.at<Vec<chType, nCh>>(Point(p.x - 1, yminus1)),
      img.at<Vec<chType, nCh>>(Point(p.x - 1, yplus1))
    ),
    Salience(
      img.at<Vec<chType, nCh>>(Point(p.x, yminus1)),
      img.at<Vec<chType, nCh>>(Point(p.x, yplus1))
    )
  );
  return (
    OrthogonalEdgeWeight * 
    ygrad + 
    MainEdgeWeight *
    Salience(
      img.at<Vec<chType, nCh>>(Point(p.x - 1, p.y)),
      img.at<Vec<chType, nCh>>(Point(p.x, p.y))
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
template<typename chType, unsigned int nCh>
double EdgeStrengthY(Mat img, Point p, SalienceFunction Salience)
{
  int xminus1 = p.x - (p.x > 0);
  int xplus1 = p.x + (p.x < img.cols - 1);

  // We use the minimum salience between the sourrounding columns at (y-1) and y
  double xgrad = MIN(
    Salience(
      img.at<Vec<chType, nCh>>(Point(xplus1, p.y)),
      img.at<Vec<chType, nCh>>(Point(xminus1, p.y))
    ),
    Salience(
      img.at<Vec<chType, nCh>>(Point(xplus1, y - 1)),
      img.at<Vec<chType, nCh>>(Point(xminus1, y - 1))
    )
  );
  return (
    OrthogonalEdgeWeight * 
    xgrad + 
    MainEdgeWeight *
    Salience(
      img.at<Vec<chType, nCh>>(Point(x, y - 1)),
      img.at<Vec<chType, nCh>>(Point(x, y))
    )
  );
}

template<typename chType, unsigned int nCh>
double EdgeStrengthSimpleX(Mat img, Point p, SalienceFunction Salience)
{
  int yminus1 = p.y - (p.y > 0);
  int yplus1 = p.y + (p.y < img.rows - 1);

  double salience = Salience<chType, nCh>(
    img.at<Vec<chType, nCh>>(Point(x, yminus1)),
    img.at<Vec<chType, nCh>>(Point(x, yplus1))
  );

  return MainEdgeWeight * salience;
}

template<typename chType, unsigned int nCh>
double EdgeStrengthSimpleY(Mat img, Point p, SalienceFunction Salience)
{
  int xminus1 = p.x - (p.x > 0);
  int xplus1 = p.x + (p.x < img.cols - 1);

  double salience = Salience(
    img.at<Vec<chType, nCh>>(Point(xminus1, y)),
    img.at<Vec<chType, nCh>>(Point(xplus1, y))
  );

  return MainEdgeWeight * salience;
}

#endif
