#ifndef TREE_FILTER_H
#define TREE_FILTER_H

#include <iostream>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <AlphaTree.h>

using cv::Vec;
using cv::Mat;
using cv::Mat_;

/**
 * Base class for filters, which apply colouring to an image based on a given
 * alpha tree and its corresponding image.
 *
 * Filters can be used any number of times at any alpha levels, though only
 * filtering at increasing levels is somewhat faster.
 */
class AbstractFilter{

	protected:
		// Alpha tree on which to base the filter
		const AlphaTree& tree;
		// Image that the alpha tree was built from
		const Mat& image;
		// Size of the given image
		const int imgsize;

	private:
		// Last alpha value at which the filter was applied
		double lambda_prev = -1;

		// Used in Union-Find to quickly determine which node determine's a pixel's colour.
		int *sets;

		const int NO_SET = -1;

		/**
		 * Resets the values for the Union-Find algorithm, such that no
		 * node is part of any set.
		 */
		void resetSets(){
			for(int i = 0; i < tree.size(); i++){
				sets[i] = NO_SET;
			}
		}

		/**
		 * Find the highest node above the given node that has an alpha
		 * level <= the given value. Also performs
		 * path compression on the sets array.
		 *
		 * @param i index of a node
		 * @param lambda maximum alpha value of the returned node.
		 * @return the index of the highest node above i, with alpha level <= lambda
		 */
		int findLambdaNode(int i, double lambda){
			int parent;
			if(sets[i] != NO_SET){
				parent = sets[i];
			}
			else{
				parent = tree[i].parent;
			}
			if(parent == BOTTOM || tree[parent].alpha > lambda)
				return i;
			int lambdaNode = findLambdaNode(parent, lambda);
			sets[i] = lambdaNode;
			return lambdaNode;
		}
		
		/**
		 * Colours the pixel at the given point in the given image, 
		 * using the colour defined at the given node in the tree.
		 *
		 * @param destination The destination image which will be coloured.
		 * 	Its required datatype and number of channels depends on
		 * 	the type of filter used.
		 * @param p the coordinates of the pixel to be coloured.
		 * @param colourNode the node in the alpha tree which should 
		 * 	determine the pixel's colour.
		 */
		virtual void colourPixel(Mat& destination, Point p, int colourNode) = 0;

	public:
		/**
		 * @param tr alpha tree on which to base the filter
		 * @param image image on which the alpha tree is based.
		 */
		AbstractFilter(const AlphaTree& tr, const Mat& image) 
			: tree(tr)
			, image(image)
			, imgsize(image.cols*image.rows){

			sets = (int *) malloc(tree.size()*sizeof(int));
			resetSets();

		}

		/**
		 * Apply the filter to the image at the given alpha value and 
		 * return the result.
		 *
		 * @param lambda the alpha level at which to apply the filter
		 * @return a filtered copy of the image.
		 */
		void filter(double lambda, Mat& destination){
			cout << "Applying filter with lambda=" << lambda << "\n";
			if(lambda < lambda_prev){
				std::cerr << "\tfiltering at lambda=" << lambda << " after filtering at lambda=" << lambda_prev << "\n"; 
				std::cerr << "\tWARNING: filtering at ascending values of lambda is faster\n";
				resetSets();
			}
			lambda_prev = lambda;

			for(int i = 0; i < imgsize; i++){
				int n = findLambdaNode(i, lambda);
				colourPixel(destination, getPoint(i, image), n);
			}
		}

		~AbstractFilter(){
			free(sets);
		}
};

/**
 * Used to create a filtered version of an image using an alpha tree, such that
 * the average colour of the pixels in a subset defined by a node in the tree 
 * is applied to every pixel in that subset.
 *
 * The destination image given to this filter should have the same datatype and 
 * number of channels as its input image.
 *
 * @tparam chType The datatype of the input image, also the required type of the
 * 	output image.
 * @tparam nCh The number of channels in the input image, also the required 
 * 	number of channels in the output image.
 */
template <typename chType, int nCh>
class AverageFilter : public AbstractFilter{
	typedef Vec<double, nCh> CalcVect;
	typedef Vec<chType, nCh> InOutVect;

	private:
		// Array of vectors representing the average colour of the pixels under each node.
		CalcVect *nodeColours;

		/**
		 * Colours each node according to the average colour of the pixels
		 * corresponding to the leaves of its subtree.
		 */
		void colourNodes(){
			// Initialize the node colours
			for(int i = imgsize; i < tree.size(); i++){
				nodeColours[i] = CalcVect::zeros();
			}

			for(int i = 0; i < tree.size(); i++){
				if(i < imgsize){
					// Leaf nodes get the colour of their pixels
					nodeColours[i] = (CalcVect) image.at<InOutVect>(getPoint(i, image));
				}else{
					// This node's children have all been processed
					nodeColours[i] /= tree[i].area;
				}
				int parent = tree[i].parent;
				if (parent == BOTTOM)
					continue;
				nodeColours[parent] += nodeColours[i]*tree[i].area;
			}
		}

		void colourPixel(Mat& destination, Point p, int colourNode){
			destination.at<InOutVect>(p) = (InOutVect) nodeColours[colourNode];
		}
		
	public:
		AverageFilter(const AlphaTree& tree, const Mat& image)
		: AbstractFilter(tree, image){
			nodeColours = (CalcVect*) malloc(tree.size()*sizeof(CalcVect));
			colourNodes();
		}

		~AverageFilter(){
			free(nodeColours);
		}
};

/**
 * Gives each pixel/leaf node a random bright colour. The colour of each node 
 * in the tree is set to the colour of the child with the largest area.
 *
 * Destination image should be 8-bit bgr, i.e. CV_8UC(3)
 */
class RandomFilter : public AbstractFilter{
	typedef Vec<uint8_t, 3> Vect3;
	typedef Mat_<Vect3> MatVect;

	private:
		// Array of bgr colour vectors
		Vect3 *nodeColours;
		
		// Keeps track of the largest area among a node's children
		int *largestChildArea;
		
		//Generates a random int in the given range
		uint8_t randInt(uint8_t min, uint8_t max){
			uint8_t r = rand() % (max-min);
			return r + min;
		}

		/**
		 * Generates a random bgr vector. In HLS colour space, the
		 * vector will have random hue, random saturation between 50 and
		 * 100%, and a lightness of 50%.
		 */
		Vect3 randColour(){
			uint8_t hue = randInt(0, 255);
			uint8_t saturation = randInt(255/2, 255);
			uint8_t lightness = 255/2;
			Vect3 colour(hue, lightness, saturation);
			// OpenCV only has colour conversion for matrices
			MatVect hls(colour);
			MatVect bgr;
			cvtColor(hls, bgr, cv::COLOR_HLS2BGR);
			return bgr.at<Vect3>(0,0);
		}

		/**
		 * Colours each pixel/leaf node with a random value, then each
		 * parent node with the colour of its child with the largest
		 * area.
		 */
		void colourNodes(int seed){
			srand(seed);
			for(int i = 0; i < tree.size(); i++){
				if(i < imgsize){
					nodeColours[i] = randColour();
				}
				int parent = tree[i].parent;
				if(parent == BOTTOM){continue;}
				if(tree[i].area > largestChildArea[parent]){
					largestChildArea[parent] = tree[i].area;
					nodeColours[parent] = nodeColours[i];
				}
			}
		}

		void colourPixel(Mat& destination, Point p, int colourNode){
			destination.at<Vect3>(p) = nodeColours[colourNode];
		}
	
	public:
		RandomFilter(const AlphaTree& tree, const Mat& image, int seed = 1)
		: AbstractFilter(tree, image){
			nodeColours = (Vect3*) malloc(tree.size()*sizeof(Vect3));
			largestChildArea = (int *) calloc(tree.size(), sizeof(int));

			colourNodes(seed);
		}

		~RandomFilter(){
			free(nodeColours);
			free(largestChildArea);
		}


};

#endif
