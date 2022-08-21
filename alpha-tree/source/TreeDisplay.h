#ifndef TREE_DISPLAY_H
#define TREE_DISPLAY_H
#include <opencv2/opencv.hpp>
#include "AlphaTree.h"
#include "TreeFilter.h"

using cv::Mat;

/**
 * Shows a simple display with a trackbar that allows the user to view the image
 * as filtered at different alpha values.
 *
 * @param image The original image. Must be displayable by openCV.
 * @param tree The alpha tree based on the image. 
 * 	Should be created with minimum alpha value of 0.
 * @param filter Alpha tree filter used to create the filtered images. 
 * 	Must generate images with the same type as `image` and with the same height.
 * @param nSlices The number of filtered images to create, 
 * 	i.e. the amount of detail defined by the trackbar.
 * @param initLambda The approximate alpha value of the initial image to display.
 *
 * @return the alpha value of the filtered image being displayed at the moment
 * 	the user closed the window.
 */
double displayTree(const Mat& image, const AlphaTree& tree, AbstractFilter& filter, int nSlices, double initLambda);

#endif
