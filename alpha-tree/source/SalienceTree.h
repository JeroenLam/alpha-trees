#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

#include <iostream>
#include <queue>
#include <vector>
#include <opencv2/opencv.hpp>

#include "DistanceFunction.h"

using cv::Mat;
using cv::Point;
using std::priority_queue;
using std::vector;

#define BOTTOM (-1)
#define LEAF_ALPHA (-1)

struct Edge{
	int p, q;
	double alpha;

	friend bool operator< (Edge const& lhs, Edge const& rhs){
		return lhs.alpha >= rhs.alpha;
	}
};


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

typedef struct SalienceNode{
	int parent;
	int area;
	double alpha;  /* alpha of flat zone */
} SalienceNode;

enum Connectivity {CN_4 = 4, CN_8 = 8};

class SalienceTree{
	private:		
		int maxSize;
		int curSize = 0;
		Mat img;
		int imgsize;
		AbstractDistanceFunction *delta;
		Connectivity cn;

		int *sets;
		SalienceNode *nodes;
		EdgeQueue queue;

		void makeEdge(Point a, Point b);
		int makeSalienceNode(double alpha);
		int findRoot(int p);
		bool isLevelRoot(int i);
		int findLevelRoot(int p);
		void mergeNodes(int p, int q);
		
		void makeEdges();
		void makeLeafNodes();
		void processEdges();
		int pruneTree();

		void buildTree(bool prune = true);

	public:
		
		SalienceTree(Mat img, AbstractDistanceFunction *delta, Connectivity cn) 
		: queue((cn/2)*img.cols*img.rows){
			this->img = img;
			this->delta = delta;
			this->cn = cn;

			imgsize = img.cols*img.rows;
			maxSize = 2*imgsize; 

			sets = (int*) malloc(imgsize*2*sizeof(int));
			nodes = (SalienceNode*) malloc((maxSize) * sizeof(SalienceNode));

			buildTree();
		};
		
		~SalienceTree(){
			free(sets);
			free(nodes);
		}

		int size();

		SalienceNode& operator[](int index);
};


Point getPoint(int index, Mat image);

#endif
