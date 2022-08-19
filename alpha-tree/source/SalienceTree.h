#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

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
enum Connectivity {CN_4 = 4, CN_8 = 8};

struct Edge{
	int p, q;
	double alpha;

	friend bool operator< (const Edge& lhs, const Edge& rhs){
		return lhs.alpha >= rhs.alpha;
	}
};


class EdgeQueue : public priority_queue<Edge, vector<Edge>>{
	public:
		EdgeQueue(int size){
			this->c.reserve(size);
		}
};

typedef struct SalienceNode{
	int parent;
	int area;
	double alpha;  /* alpha of flat zone */
} SalienceNode;

class SalienceTree{
	private:		
		//Constructor arguments
		const Mat& img;
		const AbstractDistanceFunction& delta;
		const Connectivity cn;
		const double lambdamin;
		const bool excludeTop;

		//Size-related variables
		int curSize = 0;
		const int imgsize;

		int *sets;
		SalienceNode *nodes;
		EdgeQueue queue;

		void makeEdge(Point a, Point b);
		void processEdge(Edge &edge);
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
		
		SalienceTree(const Mat& img, const AbstractDistanceFunction& delta, Connectivity cn, double lambdamin = 0, double excludeTop = false)
		: img(img)
		, delta(delta)
		, cn(cn)
		, lambdamin(lambdamin)
		, excludeTop(excludeTop)
		, imgsize(img.cols*img.rows)
		, queue((cn/2)*img.cols*img.rows){

			sets = (int*) malloc(imgsize*2*sizeof(int));
			nodes = (SalienceNode*) malloc((2*imgsize) * sizeof(SalienceNode));
			buildTree();
		};
		
		~SalienceTree(){
			free(sets);
			free(nodes);
		}

		int size() const;

		const SalienceNode& operator[] (int index) const;
};

class PointOutOfBoundsException : std::exception {
	public:
		std::string what (){ return "Point out of bounds"; }

};

Point getPoint(int index, const Mat& image);
int getIndex(Point& p, const Mat& img);

#endif
