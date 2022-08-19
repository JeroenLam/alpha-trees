#ifndef TREE_FILTER_H
#define TREE_FILTER_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <AlphaTree.h>

using cv::Vec;
using cv::Mat;

/**
 * Used to create a filtered version of an image using an alpha tree, such that
 * the average colour of the pixels in a subset defined by a node in the tree 
 * is applied to every pixel in that subset.
 *
 * The filter can be used any number of times at any alpha levels, though only
 * filtering at increasing levels is somewhat faster.
 */
template <typename chType, int nCh>
class AverageFilter{
	typedef Vec<double, nCh> Vect;

	private:
		// Alpha tree on which to base the filter
		const AlphaTree& tree;
		// Image that the alpha tree was built from
		const Mat& image;

		// Array of vectors representing the average colour of the pixels under each node.
		Vect *nodeColours;
		// Used in Union-Find to quickly determine which node determine's a pixel's colour.
		int *sets;
		// Size of the given image
		const int imgsize;
		// Last alpha value at which the filter was applied
		double lambda_prev = -1;

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
		 * Colours each node according to the average colour of the pixels
		 * corresponding to the leaves of its subtree.
		 */
		void colourNodes(){
			// Initialize the node colours
			for(int i = imgsize; i < tree.size(); i++){
				nodeColours[i] = Vect::zeros();
			}

			for(int i = 0; i < tree.size(); i++){
				if(i < imgsize){
					// Leaf nodes get the colour of their pixels
					nodeColours[i] = (Vect) image.at<Vec<chType, nCh>>(getPoint(i, image));
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

	public:

		/**
		 * @param tr alpha tree on which to base the filter
		 * @param image image on which the alpha tree is based.
		 */
		AverageFilter(const AlphaTree& tr, const Mat& image) 
			: tree(tr)
			, image(image)
			, imgsize(image.cols*image.rows){
			cout << "Creating filter:\n";

			nodeColours = (Vect*) malloc(tree.size()*sizeof(Vect));
			sets = (int *) malloc(tree.size()*sizeof(int));
			resetSets();

			cout << "\t Colouring nodes\n";
			colourNodes();
		}

		/**
		 * Apply the filter to the image at the given alpha value and 
		 * return the result.
		 *
		 * @param lambda the alpha level at which to apply the filter
		 * @return a filtered copy of the image.
		 */
		Mat filter(double lambda){
			cout << "Applying filter with lambda=" << lambda << "\n";
			if(lambda < lambda_prev){
				std::cerr << "\tfiltering at lambda=" << lambda << " after filtering at lambda=" << lambda_prev << "\n"; 
				std::cerr << "\tWARNING: filtering at ascending values of lambda is faster\n";
				resetSets();
			}
			lambda_prev = lambda;

			Mat result = Mat::zeros(image.rows, image.cols, CV_64FC(nCh));
			for(int i = 0; i < imgsize; i++){
				int n = findLambdaNode(i, lambda);
				result.at<Vect>(getPoint(i, image)) = nodeColours[n];
			}
			return result;
		}


		~AverageFilter(){
			free(sets);
			free(nodeColours);
		}
};

#endif
