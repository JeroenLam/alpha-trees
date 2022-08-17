#ifndef TREE_FILTER_H
#define TREE_FILTER_H

#include <opencv2/opencv.hpp>
#include <SalienceTree.h>
#include <iostream>


using cv::Vec;
using cv::Mat;

template <typename chType, int nCh>
class AverageFilter{

	typedef Vec<double, nCh> Vect;


	private:
		Vect *nodeColours;
		int *sets;
		Mat image;
		int imgsize;
		SalienceTree *tree;

		int NO_SET = -1;

		void printVec(Vect v){
			cout << v.val[0] << " " << v.val[1] << " " << v.val[2] << "\n";
		}

	public:
		double lambda;

		void colourNodes(){
			for(int i = imgsize; i < tree->curSize; i++){
				nodeColours[i] = Vect::zeros();
			}

			short *hasChild = (short*) calloc(tree->curSize, sizeof(short));
			for(int i = 0; i < tree->curSize; i++){
				if(i < imgsize){
					nodeColours[i] = (Vect) image.at<Vec<chType, nCh>>(getPoint(i, image));
				}
				else if(!hasChild[i]){
					continue;
				}
				int parent = tree->nodes[i].parent;
				if (parent == BOTTOM)
					continue;
				hasChild[parent] = 1;
				int area = tree->nodes[i].area;
				double areaRatio = area/((double)tree->nodes[parent].area);
				nodeColours[parent] += nodeColours[i]*areaRatio;
			}
			free(hasChild);
		}

		AverageFilter(SalienceTree *tr, Mat im){
			imgsize = im.cols*im.rows;
			nodeColours = (Vect*) malloc(tr->curSize*sizeof(Vect));
			sets = (int *) malloc(tr->curSize*sizeof(int));
			for(int i = 0; i < tr->curSize; i++){
				sets[i] = NO_SET;
			}
			image = im;
			tree = tr;

			colourNodes();
		}

		int findLambdaNode(int i, double lambda, int d){
			int parent;
			if(sets[i] != NO_SET){
				parent = sets[i];
			}
			else{
				parent = tree->nodes[i].parent;
			}
			if(parent == BOTTOM)
				return i;
			if(tree->nodes[parent].alpha > lambda)
				return i;
			int lambdaNode = findLambdaNode(parent, lambda, d+1);
			sets[i] = lambdaNode;
			return lambdaNode;
		}

		Mat filter(double lmbd){
			Mat result = Mat::zeros(image.rows, image.cols, CV_64FC(nCh));
			for(int i = 0; i < imgsize; i++){
				int n = findLambdaNode(i, lmbd, 0);
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
