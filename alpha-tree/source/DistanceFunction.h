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

template<int nCh>
class AbstractMetricFunction{
	typedef Vec<double, nCh> Vector;
	public:
		virtual double getDistance(Vector a, Vector b)= 0;
};

template<int nCh>
class MinkowskiMetricFunction : public AbstractMetricFunction<nCh>{
	typedef Vec<double, nCh> Vector;

	private:
		double exp;
		Vector wghts;

	public:
		MinkowskiMetricFunction(double exponent, Vector weights = Vector::ones()){
			exp = exponent;
			wghts = weights;
		}

		double getDistance(Vector a, Vector b){
			Vector diff, power, weightedPower;
			absdiff(a, b, diff);
			cv::pow(diff, exp, power);
			weightedPower = power.mul(wghts);
			double vecSum = sum(weightedPower)[0];
			return pow(vecSum, 1/exp);
		}
};

class AbstractDistanceFunction{
	public:
		virtual double getAlpha(Point a, Point b) = 0;
};

template<typename chType, int nCh>
class DistanceFunction : public AbstractDistanceFunction
{
private:
	Mat img;
	typedef Vec<double, nCh> Vector;
	AbstractMetricFunction<nCh> *metr;
public:
	DistanceFunction(Mat image, AbstractMetricFunction<nCh> *metric){
		img = image;
		metr = metric;
	}

	double getAlpha(Point a, Point b){
		Vector aVec = (Vector) img.at<Vec<chType, nCh>>(a);
		Vector bVec = (Vector) img.at<Vec<chType, nCh>>(b);
		return metr->getDistance(aVec, bVec);
	}
};

#endif
