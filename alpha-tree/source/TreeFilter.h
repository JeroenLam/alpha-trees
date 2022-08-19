#ifndef TREE_FILTER_H
#define TREE_FILTER_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <SalienceTree.h>

using cv::Vec;
using cv::Mat;

template <typename chType, int nCh>
class AverageFilter{
	typedef Vec<double, nCh> Vect;


	private:
		const SalienceTree& tree;
		const Mat& image;

		Vect *nodeColours;
		int *sets;
		const int imgsize;
		double lambda_prev = -1;

		int NO_SET = -1;

	public:

		AverageFilter(const SalienceTree& tr, const Mat& image) 
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

		void resetSets(){
			for(int i = 0; i < tree.size(); i++){
				sets[i] = NO_SET;
			}
		}

		void colourNodes(){
			for(int i = imgsize; i < tree.size(); i++){
				nodeColours[i] = Vect::zeros();
			}

			for(int i = 0; i < tree.size(); i++){
				if(i < imgsize){
					nodeColours[i] = (Vect) image.at<Vec<chType, nCh>>(getPoint(i, image));
				}else{
					nodeColours[i] /= tree[i].area;
				}
				int parent = tree[i].parent;
				if (parent == BOTTOM)
					continue;
				nodeColours[parent] += nodeColours[i]*tree[i].area;
			}
		}

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

		~AverageFilter(){
			free(sets);
			free(nodeColours);
		}
};

#endif
