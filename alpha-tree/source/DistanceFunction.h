#ifndef DISTANCE_FUNCTION_H
#define DISTANCE_FUNCTION_H

#include <opencv2/opencv.hpp>

#include "gabor/gabor.h"

using cv::Point;
using cv::Vec;
using cv::Mat;
using cv::absdiff;
using cv::sum;

using std::cout;

/**
 * Base class for metric functions, which define a distance between vectors.
 *
 * @tparam nCh number of elements in the vectors on which the distance is defined.
 * 	I.e. the number of channels in the image.
 */
template<int nCh>
class AbstractMetricFunction{
	typedef Vec<double, nCh> Vector;
	public:
		/**
		 * Calculates the distance between the given vectors.
		 * @param a, b Any vector in a space with nCh dimensions.
		 */
		virtual double getDistance(Vector& a, Vector& b) const = 0;
};

/**
 * Defines minkowski distance, also known as Lp distance.
 * MinkowskiMetricFunction<3> m(2); defines euclidean distance in 3D space.
 * MinkowskiMetricFunction<2> m(1); defines manhattan distance in 2D space. 
 */
template<int nCh>
class MinkowskiMetricFunction : public AbstractMetricFunction<nCh>{
	typedef Vec<double, nCh> Vector;

	private:
		const double exponent;
		const Vector weights;

	public:
		/**
		 * @param exponent the power to which the absolute differences 
		 * 	between vector entries is raised, and the root applied
		 * 	to their sum.
		 * @param weights a vector of weights applied to each dimension.
		 */
		MinkowskiMetricFunction(double exponent, Vector weights = Vector::ones())
		: exponent(exponent)
		, weights(weights){}

		/**
		 * @param a, b the vectors whose distance from each other should be calculated.
		 * @returns the distance between a and b as defined by this metric.
		 */
		double getDistance(Vector& a, Vector& b) const{
			Vector diff, power, weightedPower;
			absdiff(a, b, diff);
			cv::pow(diff, exponent, power);
			weightedPower = power.mul(weights);
			double vecSum = sum(weightedPower)[0];
			return pow(vecSum, 1/exponent);
		}
};

/**
 * Base class for distance functions. Unlike metric functions that only define
 * distance between vectors, distance functions define a distance between the 
 * values of two pixels in a given image. This allows distance function to take
 * into consideration such things as the texture around the two pixels, the 
 * image's statistics etc.
 */
class AbstractDistanceFunction{
	public:
		/**
		 * Get the alpha value of the edge between the pixels located at
		 * the given points.
		 *
		 * @param a, b a 2D point locating a pixel in the image.
		 * @return the alpha value of the edge between the pixels at a and b.
		 */
		virtual double getAlpha(Point& a, Point& b) const = 0;
};

/**
 * The most basic distance function, which simply applies its metric function to
 * the given image.
 * 
 * @tparam chType the datatype of the image's pixels.
 * @tparam nCh the number of channels in the image.
 */
template<typename chType, int nCh>
class DistanceFunction : public AbstractDistanceFunction{
	typedef Vec<double, nCh> Vector;
	protected:
		// Image on which the distance function is defined
		const Mat& image;

		// Metric function applied to the pixels' values
		const AbstractMetricFunction<nCh>& metric;
	public:
		/**
		 * @param image the image on which the distance function should be defined
		 * @param metric the metric function to be applied to the pixels' values
		 */
		DistanceFunction(const Mat& image, const AbstractMetricFunction<nCh>& metric)
		: image(image)
		, metric(metric){}

		double getAlpha(Point& a, Point& b) const{
			Vector aVec = (Vector) image.at<Vec<chType, nCh>>(a);
			Vector bVec = (Vector) image.at<Vec<chType, nCh>>(b);
			return metric.getDistance(aVec, bVec);
		}
};

template<typename chType, int nCh>
class SimpleGaborDistanceFunction : public DistanceFunction<chType, nCh>{
		typedef Vec<double, nCh> Vector;
		private:
			Mat filteredImage;
			const int nAngles;
			const double matchFactor;
			const bool binaryMatch;
			const RidgeType ridgeType;
		public:
			SimpleGaborDistanceFunction(
					const Mat& image, 
					const AbstractMetricFunction<nCh>& metric, 
					const Mat& image_grey, 
					double lambda, 
					double gamma,
					double sigma,
					int nAngles, 
					double thresholdFactor,
					double matchFactor,
					RidgeType ridgeType = LIGHT,
					bool binaryMatch = true
					)
			: DistanceFunction<chType, nCh>(image, metric)
			, nAngles(nAngles)
			, matchFactor(matchFactor)
			, binaryMatch(binaryMatch)
			, ridgeType(ridgeType){
				mainDirections(image_grey, lambda, gamma, sigma, nAngles, thresholdFactor, filteredImage, ridgeType);
			}

			SimpleGaborDistanceFunction(
					const Mat& image, 
					const AbstractMetricFunction<nCh>& metric, 
					ColorConversionCodes conversionCode, 
					double lambda, 
					double gamma,
					double sigma,
					int nAngles, 
					double thresholdFactor,
					double matchFactor,
					RidgeType ridgeType = LIGHT,
					bool binaryMatch = true
					)
			: DistanceFunction<chType, nCh>(image, metric)
			, nAngles(nAngles)
			, matchFactor(matchFactor)
			, binaryMatch(binaryMatch)
			, ridgeType(ridgeType){
				Mat image_grey;
				cvtColor(image, image_grey, conversionCode);
				mainDirections(image_grey, lambda, gamma, sigma, nAngles, thresholdFactor, filteredImage, ridgeType);
			}

			const Mat& getFilteredImage(){
				return filteredImage;
			}

			void getGreyFilteredImage(Mat &destination){
				int max = nAngles;
				int min = ridgeType == BOTH ? -nAngles : 0;
				int multiply = 255/(max - min);
				int add = ridgeType == BOTH ? 255/2 : 0;
				filteredImage.convertTo(destination, CV_8U, multiply, add);
			}
			
			double getAlpha(Point& a, Point& b) const{
				double alpha = DistanceFunction<chType, nCh>::getAlpha(a, b);
				int8_t dirA = filteredImage.at<int8_t>(a); 
				int8_t dirB = filteredImage.at<int8_t>(b); 
				if(dirA && dirA == dirB){
					alpha *= matchFactor;
				}
				return alpha;
			}
};

#endif
