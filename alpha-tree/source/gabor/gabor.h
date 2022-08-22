#ifndef GABOR_H 
#define GABOR_H
#include <opencv2/opencv.hpp>

using namespace cv;

enum RidgeType{LIGHT, DARK, BOTH};

void mainDirections(
		const Mat& image_grey,
		double lambda,
		double gamma,
		double sigma,
		int n_dirs,
		double threshold_factor,
		Mat& destination,
		RidgeType ridgeType
		);

#endif
