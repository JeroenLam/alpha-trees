#include "TreeDisplay.h"

using std::vector;
using std::string;
 
using cv::namedWindow;
using cv::createTrackbar;
using cv::getTrackbarPos;
using cv::setTrackbarPos;
using cv::imshow;
using cv::waitKey;

const string WINDOW_NAME = "Display Window";
const string TRACKBAR_NAME = "Alpha Value";

/**
 * The data required by the trackbar callback function in order to display the 
 * correct image an alpha value, as well as to keep track of the last value 
 * displayed.
 */
struct DisplayData{
	// Vector of filtered images with increasing alpha values
	vector<Mat> images;
	// Alpha values the images were filtered at
	vector<double> alphaValues;
	// Used to save the filtered image after the display is closed.
	double lastAlpha; 
};

/**
 * Trackbar callback function which is called when the trackbar's value changes.
 * Displays the image corresponding to the alpha value associated with the slider
 * position, and prints said alpha value.
 *
 * @param data A pointer to the DisplayData object created in the displayTree function.
 */
void onChange(int, void* data) {
	int pos = getTrackbarPos(TRACKBAR_NAME, WINDOW_NAME);
	if(pos < 0){return;}
	DisplayData* displayData = (DisplayData*) data;
	double alpha = displayData->alphaValues[pos];
	displayData->lastAlpha = alpha;
	Mat out = displayData->images[pos];
	cout << "\r                                  ";
	cout << "\rDisplaying with alpha = " << alpha;
	cout.flush();
	imshow(WINDOW_NAME, out);
}

/**
 * Generates a range of alpha values to filter at. Currently it just divides
 * up the range from 0 to the maximum alpha value in the tree, which results
 * in approximately the latter half of the values not being very interesting.
 * Generating more interesting alpha values for this might be a job for an
 * alpha tree of an alpha tree.
 *
 * @param tree Alpha tree whose alpha values the range should be based on.
 * @param nSlices The number of alpha values to generate
 */
vector<double> alphaValueRange(const AlphaTree& tree, int nSlices){

	vector<double> values;
	nSlices = nSlices < 2 ? 0 : nSlices - 2;
	values.push_back(0);
	double step = (tree[tree.size()-1].alpha)/(nSlices+1);

	for(int i = 1; i <= nSlices; i++){
		values.push_back(i*step);
	}

	values.push_back(tree[tree.size()-1].alpha);
	cout << "SIZE " << nSlices << "\n";
	return values;
}

double displayTree(const Mat& image, const AlphaTree& tree, AbstractFilter& filter, int nSlices, double initLambda = 0){
	vector<double> alphaValues = alphaValueRange(tree, nSlices);
	nSlices = alphaValues.size();
	vector<Mat> outImages;

	// Variables used to find which slice comes closest to displaying initLambda
	double lamdaDiff = alphaValues[nSlices-1];
	int displaySlice = 0;
	for(int i = 0; i < nSlices; i++){
		cout << "\rGenerating image " << (i+1) << "/" << nSlices;
		cout.flush();

		// Filter image and attach it to the original
		Mat out;
		Mat filtered(image.rows, image.cols, CV_8UC(3));
		filter.filter(alphaValues[i], filtered, false);
		hconcat(image, filtered, out);
		outImages.push_back(out);

		// Check if this image should be the first to be displayed
		double diff = abs(alphaValues[i] - initLambda);
		if( diff < lamdaDiff){
			displaySlice = i;
			lamdaDiff = diff;
		}
	}
	cout << "\n";

	DisplayData data{.images = outImages, .alphaValues = alphaValues, .lastAlpha = alphaValues[displaySlice]};

	namedWindow(WINDOW_NAME);
	createTrackbar(TRACKBAR_NAME, WINDOW_NAME, NULL, nSlices-1, onChange, (void*) &data);
	setTrackbarPos(TRACKBAR_NAME, WINDOW_NAME, displaySlice);
	imshow(WINDOW_NAME, outImages[displaySlice]);

	int key = 0;
	// Exit with Esc, Space, Enter or Backspace
	while (key != 27 && key != 32 && key != 13 && key != 8) {
	   key = waitKey(500);
	}
	cout << "\n";
	return data.lastAlpha;
}
