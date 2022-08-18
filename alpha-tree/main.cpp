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

#include <SalienceTree.h>
#include <TreeFilter.h>

using namespace std;
using cv::Mat;

int main(int argc, char *argv[]){
	string imgfname, outfname = "out.ppm";
	int r;
	unsigned long i;
	clock_t start;
	struct tms tstruct;
	long tickspersec = sysconf(_SC_CLK_TCK);
	float musec;

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

	cout << "Filtering image '"<< imgfname << "' using attribute area with lambda=" << lambda << "\n";
	cout << "Image: Width=" << image.cols << "Height=" << image.rows << "\n";

	start = times(&tstruct);
	// create the actual alpha tree
	MinkowskiMetricFunction<3> metric(2);
	DistanceFunction<uint8_t, 3> delta(image, &metric);
	SalienceTree tree(image, &delta, CN_4);
	AverageFilter<uint8_t, 3> filter(tree, image);
	Mat out = filter.filter(lambda);
	Mat out2 = filter.filter(lambda*2);

	musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

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
