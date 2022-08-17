#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

#include <iostream>

#include "../util/common.h"
#include "EdgeQueue.h"
#include "DistanceFunction.h"
#include <opencv2/opencv.hpp>

#define Par(tree, p) LevelRoot(tree, tree->nodes[p].parent)

using cv::Mat;
using cv::Point;

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
