#ifndef DISTANCE_FUNCTION_H
#define DISTANCE_FUNCTION_H

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
		virtual double getDistance(Vector& a, Vector& b) const = 0;
};

template<int nCh>
class MinkowskiMetricFunction : public AbstractMetricFunction<nCh>{
	typedef Vec<double, nCh> Vector;

	private:
		const double exponent;
		const Vector weights;

	public:
		MinkowskiMetricFunction(double exponent, Vector weights = Vector::ones())
		: exponent(exponent)
		, weights(weights){}

		double getDistance(Vector& a, Vector& b) const{
			Vector diff, power, weightedPower;
			absdiff(a, b, diff);
			cv::pow(diff, exponent, power);
			weightedPower = power.mul(weights);
			double vecSum = sum(weightedPower)[0];
			return pow(vecSum, 1/exponent);
		}
};


class AbstractDistanceFunction{
	public:
		virtual double getAlpha(Point& a, Point& b) const = 0;
};

template<typename chType, int nCh>
class DistanceFunction : public AbstractDistanceFunction{
private:
	typedef Vec<double, nCh> Vector;

	const Mat& image;
	const AbstractMetricFunction<nCh>& metric;
public:
	DistanceFunction(const Mat& image, const AbstractMetricFunction<nCh>& metric)
	: image(image)
	, metric(metric){}

	double getAlpha(Point& a, Point& b) const{
		Vector aVec = (Vector) image.at<Vec<chType, nCh>>(a);
		Vector bVec = (Vector) image.at<Vec<chType, nCh>>(b);
		return metric.getDistance(aVec, bVec);
	}
};

#endif
