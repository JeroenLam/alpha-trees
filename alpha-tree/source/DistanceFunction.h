#ifndef DISTANCE_FUNCTION_H
#define DISTANCE_FUNCTION_H

#include <math.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Vec;
using cv::Mat;
using cv::absdiff;
using cv::sum;

using std::cout;

template<int nCh, int exponent>
double minkowski(Vec<double, nCh> a, Vec<double, nCh> b){
	Vec<double, nCh> diff, power;
	absdiff(a, b, diff);
	cv::pow(diff, exponent, power);
	double vecSum = sum(power)[0];
	return pow(vecSum, 1/((double)exponent));
}

class AbstractDistanceFunction{
	public:
		virtual double getDistance(Point a, Point b) = 0;
};

template<typename chType, int nCh>
class DistanceFunction : public AbstractDistanceFunction
{
private:
	Mat img;
	typedef Vec<double, nCh> Vector;
	double (*metric)(Vector a, Vector b);
public:
	DistanceFunction(Mat image, double (*distMetric)(Vector a, Vector b)){
		img = image;
		metric = distMetric;
	}

	double getDistance(Point a, Point b){
		Vector aVec = (Vector) img.at<Vec<chType, nCh>>(a);
		Vector bVec = (Vector) img.at<Vec<chType, nCh>>(b);
		return metric(aVec, bVec);
	}
};

#endif
