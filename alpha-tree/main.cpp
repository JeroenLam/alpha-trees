#include <sys/times.h>
#include <unistd.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <AlphaTree.h>
#include <TreeFilter.h>
#include <TreeDisplay.h>

using namespace std;
using cv::Mat;

int main(int argc, char *argv[]){
	struct tms tstruct;
	long tickspersec = sysconf(_SC_CLK_TCK);

	// Check if the right amount of arguments are provided and set variables according to them
	if (argc < 3){
		printf("Usage: %s <input image> <lambda>  [output image] \n", argv[0]);
		exit(0);
	}

	string imgfname = argv[1];

	double lambda = atof(argv[2]);

	string outfname = "out.ppm";
	if (argc > 3)
		outfname = argv[3];

	// Read the input image
	Mat image = cv::imread(imgfname, 1);
	if (image.empty()){
		std::cerr << "Failed to read input image!\n";
		return (-1);
	}

	cout << "Filtering image '"<< imgfname << "' using attribute area with lambda=" << lambda << "\n";
	cout << "Image: Width=" << image.cols << "Height=" << image.rows << "\n";

	clock_t start = times(&tstruct);

	// create the actual alpha tree
	MinkowskiMetricFunction<3> metric(2);
	//DistanceFunction<uint8_t, 3> delta(image, metric);
	SimpleGaborDistanceFunction<uint8_t, 3> delta(
			image, 
			metric, 
			COLOR_BGR2GRAY,
			4, // lambda: wavelength of sine function, i.e. ridge width
			0.5, // gamma: aspect ratio of the gaussian function
			1, // sigma: standard deviation of gaussian function
			32, // nAngles: number of axes in the filter bank BUG: nAngles=8 or 128messes with things for some reason
			8, // thresholdFactor
			0.5,  // matchFactor
			LIGHT, //ridgeType
			false  //binaryMatch: true is not yet implemented
		);
	AlphaTree tree(image, delta, CN_8);
	//AverageFilter<uint8_t, 3> filter(tree, image);
	RandomFilter filter(tree, image);


	Mat gaborFiltered;
	delta.getGreyFilteredImage(gaborFiltered);
	cvtColor(gaborFiltered, gaborFiltered, COLOR_GRAY2BGR);
	vector<Mat> compareImages{gaborFiltered};
	double outLambda = displayTree(image, compareImages, tree, filter, 100, lambda, 25);
	Mat out = Mat::zeros(image.rows, image.cols, CV_8UC(3));
	Mat out2 = Mat::zeros(image.rows, image.cols, image.type());
	filter.filter(outLambda, out);
	filter.filter(outLambda*2, out2);

	float musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

	printf("wall-clock time: %f s\n", musec);

	musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

	printf("wall-clock time: %f s\n", musec);

	bool success = cv::imwrite(outfname, out);
	success = success && cv::imwrite("out2.ppm", out2);
	if(!success)
		cerr << "Could not write image to " << outfname << "\n"; 

	cout << "Filtered image written to '" << outfname << "'\n";

	return (0);
} /* main */
