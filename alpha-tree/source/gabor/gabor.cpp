#include <opencv2/opencv.hpp>
#include "gabor.h"

using std::vector;
using std::cout;
using std::abs;

using cv::Mat;
using cv::getGaborKernel;
using cv::split;
using cv::abs;
using cv::sum;
using cv::threshold;
using cv::THRESH_TOZERO;
using cv::countNonZero;
using cv::Size;

vector<Mat> gaborFilter(double lambda, double gamma, double sigma, double theta, double psi, const Mat& image){
	int k_size = (int) lambda*8;
	Mat kernel = getGaborKernel(Size(k_size, k_size), sigma, theta, lambda, gamma, psi);

	Mat positive;
	Mat negative;
	threshold(kernel, positive, 0, 0, THRESH_TOZERO);
	threshold(kernel*-1, negative, 0, 0, THRESH_TOZERO);
	double ratio = sum(positive)[0]/sum(negative)[0];
	
	kernel = positive - ratio*negative;

	vector<Mat> channels(image.channels());
	split(image, channels);

	vector<Mat> output(image.channels());
	for(int i = 0; i < channels.size(); i++){
		filter2D(channels[i], output[i], CV_64F, kernel);
	}

	return output;
}

Mat greyScaleGaborFilter(double lambda, double gamma, double sigma, double theta, double psi, const Mat& image_grey){
	vector<Mat> filtered = gaborFilter(lambda, gamma, sigma, theta, psi, image_grey);
	return filtered[0];
}

void mainDirections(
		const Mat& image_grey,
		double lambda,
		double gamma,
		double sigma,
		int n_dirs,
		double threshold_factor,
		Mat& destination,
		RidgeType ridgeType
		){
	vector<Mat> responses(n_dirs);
	Mat sum_resp = Mat::zeros(image_grey.rows, image_grey.cols, CV_64F);
	int nonZero = 0;
	for(int i = 0; i < n_dirs; i++){
		double theta = i*(CV_PI/n_dirs);
		responses[i] = greyScaleGaborFilter(lambda, gamma, sigma, theta, 0, image_grey);
		if(ridgeType == DARK){responses[i]*=-1;}
		if(ridgeType != BOTH){threshold(responses[i], responses[i], 0, 0, THRESH_TOZERO);}
		sum_resp += abs(responses[i]);
		nonZero += countNonZero(responses[i]);
	}
	double mean_resp = sum(sum_resp)[0]/nonZero;
	double thresh = threshold_factor*mean_resp;

	destination = Mat::zeros(image_grey.rows, image_grey.cols, CV_8S);
	for(int i = 0; i < image_grey.rows; i++){
		for(int j = 0; j < image_grey.cols; j++){
			double max_resp = 0;
			for(int k = 0; k < n_dirs; k++){
				double resp = responses[k].at<double>(i,j);
				if(abs(resp) >= thresh && abs(resp) > max_resp){
					max_resp = abs(resp);
					destination.at<int8_t>(i,j) = (k+1)*((resp>0) - (resp<0));
				}
			}
		}
	}
}

