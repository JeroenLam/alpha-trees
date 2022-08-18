#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>

#include <opencv2/opencv.hpp>

#include <common.h>
#include <DistanceMeasures.h>
#include <EdgeQueue.h>
#include <SalienceTree.h>
#include <TreeFilter.h>

using namespace std;
using cv::Mat;

double RGBweight[3] = {0.5, 0.5, 0.5};
double MainEdgeWeight = 1.0;
double OrthogonalEdgeWeight = 1.0;
double SalienceRange[2] = {0, 10000};

// variables
//double omegafactor = 200000;

int main(int argc, char *argv[])
{
	int width, height, size;

	string imgfname, outfname = "out.ppm";
	int r;
	unsigned long i;
	clock_t start;
	struct tms tstruct;
	long tickspersec = sysconf(_SC_CLK_TCK);
	float musec;
	SalienceTree *tree;

	// Check if the right amount of arguments are provided and set variables according to them
	if (argc < 3)
	{
	printf("Usage: %s <input image> <lambda>  [omegafactor] [output image] \n", argv[0]);
	exit(0);
	}

	imgfname = argv[1];

	int lambda = atoi(argv[2]);
	if (argc > 3)
	double omegafactor = atof(argv[3]);

	if (argc > 4)
	outfname = argv[4];

	// Read the input image
	Mat image = cv::imread(imgfname, 1);
	if (image.empty()){
		std::cerr << "Failed to read input image!\n";
		return (-1);
	}
	size = image.cols*image.rows;

	cout << "Filtering image '"<< imgfname << "' using attribute area with lambda=" << lambda << "\n";
	cout << "Image: Width=" << image.cols << "Height=" << image.rows << "\n";

	printf("Data read, start filtering.\n");
	start = times(&tstruct);
	// create the actual alpha tree
	MinkowskiMetricFunction<3> metric(2);
	DistanceFunction<uint8_t, 3> delta(image, &metric);
	tree = MakeSalienceTree(image, &delta, CN_4);
	//tree = MakeSalienceTree(gval, width, height, (double)lambda);
	AverageFilter<uint8_t, 3> filter(tree, image);
	Mat out = filter.filter(lambda);
	Mat out2 = filter.filter(lambda*2);

	musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

	printf("wall-clock time: %f s\n", musec);
	// apply what we have found in the alpha tree creation to the out image
	// here colors and areas are created etc.
	// SalienceTreeAreaFilter(tree,out,lambda);
	//SalienceTreeSalienceFilter(tree, out, (double)lambda);
	// SalienceTreeColorMapFilter(tree, out, (double)lambda);

	musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

	printf("wall-clock time: %f s\n", musec);

	bool success = cv::imwrite(outfname, out);
	success = success && cv::imwrite("out2.ppm", out2);
	if(!success)
		cerr << "Could not write image to " << outfname << "\n"; 

	cout << "Filtered image written to '" << outfname << "'\n";

	return (0);
} /* main */
