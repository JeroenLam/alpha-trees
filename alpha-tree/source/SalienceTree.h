#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

#include <iostream>

#include "../util/common.h"
#include "EdgeQueue.h"
#include "DistanceFunction.h"
#include <opencv2/opencv.hpp>

#define Par(tree, p) LevelRoot(tree, tree->nodes[p].parent)

using cv::Mat;

class PointOutOfBoundsException : std::exception {
	public:
		std::string what (){ return "Point out of bounds"; }

};

typedef struct SalienceNode
{
  int parent;
  int area;
  boolean filtered; /* indicates whether or not the filtered value is OK */
  Pixel outval;  /* output value after filtering */
  double alpha;  /* alpha of flat zone */
  double sumPix[3];
  Pixel minPix;
  Pixel maxPix;
} SalienceNode;

typedef struct SalienceTree
{
  int maxSize;
  int curSize;
  SalienceNode *nodes;
} SalienceTree;

enum Connectivity {CN_4, CN_8};
double const LEAF_ALPHA = -1;


SalienceTree *MakeSalienceTree(Mat img, AbstractDistanceFunction *delta, Connectivity cn);
void DeleteTree(SalienceTree *tree);
int Depth(SalienceTree *tree, int p);

#endif
