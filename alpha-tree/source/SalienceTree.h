#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

#include <iostream>
#include <queue>
#include <vector>
#include <opencv2/opencv.hpp>

#include "../util/common.h"
#include "DistanceFunction.h"

using cv::Mat;
using cv::Point;
using std::priority_queue;
using std::vector;

typedef struct Edge
{
	int p, q;
	double alpha;

	friend bool operator< (Edge const& lhs, Edge const& rhs){
		return lhs.alpha >= rhs.alpha;
	}
} Edge;


class EdgeQueue : public priority_queue<Edge, vector<Edge>>{
	public:
		EdgeQueue(int size){
			this->c.reserve(size);
		}
};

class PointOutOfBoundsException : std::exception {
	public:
		std::string what (){ return "Point out of bounds"; }

};

typedef struct SalienceNode
{
  int parent;
  int area;
  double alpha;  /* alpha of flat zone */
} SalienceNode;

typedef struct SalienceTree
{
  int maxSize;
  int curSize;
  SalienceNode *nodes;
} SalienceTree;

enum Connectivity {CN_4 = 4, CN_8 = 8};
double const LEAF_ALPHA = -1;


SalienceTree *MakeSalienceTree(Mat img, AbstractDistanceFunction *delta, Connectivity cn);
void DeleteTree(SalienceTree *tree);
Point getPoint(int index, Mat image);
int Depth(SalienceTree *tree, int p);

#endif
