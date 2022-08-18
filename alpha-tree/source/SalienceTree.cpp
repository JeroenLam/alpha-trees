#include "SalienceTree.h"
#include "DistanceFunction.h"
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Mat;
using std::priority_queue;

// i(x,y) = y*width + x
// x,y (i) = i % width, i//width

Point getPoint(int index, Mat img){
	int x = index % img.cols;
	int y = index / img.cols;
	if(y < 0 || x < 0 || y >= img.rows || x >= img.cols)
		throw PointOutOfBoundsException();
	return Point(x,y);
}

int getIndex(Point p, Mat img){
	if(p.y < 0 || p.x < 0 || p.y >= img.rows || p.x >= img.cols)
		throw PointOutOfBoundsException();
	return p.y*img.cols + p.x;
}

void makeEdge(Mat img, Point a, Point b, EdgeQueue& queue, AbstractDistanceFunction *delta){
	int a_index, b_index;
	try{
		a_index = getIndex(a, img);
		b_index = getIndex(b, img);
	} catch(PointOutOfBoundsException e){
		return;
	}
	
	Edge e = {.p = a_index, .q = b_index, .alpha = delta->getAlpha(a,b)}; 
	queue.push(e);
}

void makeEdges(Mat img, Connectivity cn, EdgeQueue& queue, AbstractDistanceFunction *delta){
	for(int x = 0; x < img.cols; x++){
		for(int y = 0; y < img.rows; y++){
			Point p = Point(x,y);
			makeEdge(img, p, Point(x+1,y), queue, delta);
			makeEdge(img, p, Point(x,y+1), queue, delta);

			if (cn == CN_8){
				makeEdge(img, p, Point(x-1, y+1), queue, delta);
				makeEdge(img, p, Point(x+1, y+1), queue, delta);
			}
		}
	}
}

void makeLeafNodes(SalienceTree *tree, int *sets, int imgsize){
	for(int i = 0; i < imgsize; i++){ 
		tree->nodes[i].parent = BOTTOM;
		sets[i] = BOTTOM;
		tree->nodes[i].alpha = LEAF_ALPHA;
		tree->nodes[i].area = 1;
	}
}

/**
 * @brief Free memory allocated for a given Salience Tree
 * 
 * @param tree Tree to free the memory of
 */
void DeleteTree(SalienceTree *tree)
{
  free(tree->nodes);
  free(tree);
}

/**
 * @brief Create a Salience Node object in a given tree
 * 
 * @param tree Tree to which the node will be added
 * @param sets 
 * @param alpha The nodes alpha level
 * @return int Index of the new node
 */
int makeSalienceNode(SalienceTree *tree, int *sets, double alpha){
	// node is the next free spot in the tree (pointer arithmetics)
	SalienceNode *nodes = tree->nodes + tree->curSize;
	int result = tree->curSize;
	tree->curSize++;
	nodes->alpha = alpha;
	nodes->parent = BOTTOM;
	sets[result] = BOTTOM;
	return result;
}

/**
 * @brief Finds the root of a set of nodes. Also performs path compression.
 *
 * @param sets The array containing the set graph
 * @param p the index in the sets array of the node whose set's root needs to be found
 */
int findRoot(int *sets, int p){
	if(sets[p] == BOTTOM){return p;}
	int root = findRoot(sets, sets[p]);
	sets[p] = root;
	return root;
}

/**
 * @brief Determine if a node at a given index is the root of its level of the tree.
 * A node is considered the level root if it has the same alpha level as its parent
 * or does not have a parent.
 * 
 * @param tree Tree to check
 * @param i Index of the node
 * @return true if the node is at root level
 * @return false if the node is not at root level
 */
bool isLevelRoot(SalienceTree *tree, int i){
	int parent = tree->nodes[i].parent;
	if (parent == BOTTOM){return true;}
	return (tree->nodes[i].alpha != tree->nodes[parent].alpha);
}

/**
 * @brief Finds the root node of the alpha level of a given node.
 * 
 * @param tree Tree to search
 * @param p Node to find the level root of
 * @return int Index of the level root
 */
int findLevelRoot(SalienceTree *tree, int p){
	if(isLevelRoot(tree, p)){return p;}
	int root = findLevelRoot(tree, tree->nodes[p].parent);
	tree->nodes[p].parent = root;
	return root;
}

void mergeNodes(SalienceTree *tree, int *sets, int p, int q){
	tree->nodes[q].parent = p;
	sets[q] = p;
	tree->nodes[p].area += tree->nodes[q].area;
}

void processEdges(SalienceTree *tree, EdgeQueue& queue, int *sets)
{
	while (!queue.empty()){
		// deque the current edge and temporarily store its values
		Edge edge = queue.top();
		queue.pop();
		int root1 = findRoot(sets, edge.p);
		int root2 = findRoot(sets, edge.q);

		if (root1 == root2){continue;}
		if (root1 < root2){std::swap(root1, root2);}

		if (tree->nodes[root1].alpha < edge.alpha){
			// if the higher node has a lower alpha level than the edge
			// we combine the two nodes in a new salience node
			int new_root = makeSalienceNode(tree, sets, edge.alpha);
			mergeNodes(tree, sets, new_root, root1);
			mergeNodes(tree, sets, new_root, root2);
		}
		else{
			// otherwise we add the lower node to the higher node
			mergeNodes(tree, sets, root1, root2);
		}
	}
}

int pruneTree(SalienceTree *tree, int imgsize){
	bool *keepNode = (bool*) calloc(tree->curSize, sizeof(bool));
	bool *freeIndex = (bool*) calloc(tree->curSize, sizeof(bool));
	int *nodeMap = (int*) malloc(tree->curSize*sizeof(int));
	int nextFree = -1;
	int pruneAmount = 0;

	for(int i = 0; i < tree->curSize; i++){
		if(i < imgsize){keepNode[i] = true;} //Keep all leaf nodes
		else if(!keepNode[i]){ //Node has not been claimed as parent
			if(nextFree < 0){nextFree = i;}
			freeIndex[i] = true;
			pruneAmount++;
			continue;
		}

		int parent = tree->nodes[i].parent;
		if (parent != BOTTOM){
			int levelParent = findLevelRoot(tree, parent);
			tree->nodes[i].parent = levelParent;
			keepNode[levelParent] = true;
		}

		nodeMap[i] = i;
		if(nextFree >= 0){
			nodeMap[i] = nextFree;
			freeIndex[nextFree] = false;
			freeIndex[i] = true;
			for(; !freeIndex[nextFree] && nextFree < i; nextFree++){};
		}
	}

	free(freeIndex);

	if(nextFree < 0){
		free(keepNode);
		free(nodeMap);
		return 0;
	}

	SalienceNode *prunedNodes = (SalienceNode*) malloc((tree->curSize - pruneAmount)*sizeof(SalienceNode));
	for(int i = 0; i < tree->curSize; i++){
		if(!keepNode[i]){continue;}

		SalienceNode node = tree->nodes[i];
		node.parent = nodeMap[node.parent];
		prunedNodes[nodeMap[i]] = node;
		
	}
	tree->nodes = prunedNodes;
	tree->curSize = tree->curSize - pruneAmount;

	free(keepNode);
	free(nodeMap);
	
	return pruneAmount;
}



SalienceTree *MakeSalienceTree(Mat img, AbstractDistanceFunction *delta, Connectivity cn)
{
	int imgsize = img.cols*img.rows;

	// Priority queue holding edges between adjacent pixels, minimum edge weight has highest priority
	EdgeQueue queue((cn/2)*imgsize);

	// Array representing the sets for the Union-Find algorithm. 
	// Used to quickly look up the root node of the sub alpha tree a certain node is in.
	// sets[a] == b means that a and b are in the same set. 
	// sets[a] == BOTTOM means a is the root/representative element of the set
	int *sets = (int*) malloc(imgsize * 2 * sizeof(int));

	// Initialization of the SalienceTree object
	SalienceTree *tree = (SalienceTree*) malloc(sizeof(SalienceTree));
	tree->maxSize = 2 * imgsize; /* potentially twice the number of nodes as pixels exist*/
	tree->curSize = imgsize;     /* first imgsize taken up by pixels */
	// Array holding the nodes in the alpha tree. 
	// If the image has n pixels, the first n nodes are leaf nodes and correspond to the image's pixels.
	tree->nodes = (SalienceNode*) malloc((tree->maxSize) * sizeof(SalienceNode));
	assert(tree != NULL);
	assert(tree->nodes != NULL);

	cout << "Creating Edges\n";
	makeEdges(img, cn, queue, delta);
	cout << "Creating leaf nodes\n";
	makeLeafNodes(tree, sets, imgsize);
	cout << "Building alpha tree\n";
	processEdges(tree, queue, sets);
	cout << "Pruning alpha tree\n";
	int nodesPruned = pruneTree(tree, imgsize);
	cout << nodesPruned << " nodes pruned\n";

	free(sets);
	return tree;
}

int Depth(SalienceTree *tree, int p)
{
  int depth = 0;
  while (tree->nodes[p].parent != BOTTOM)
  {
    depth++;
    p = tree->nodes[p].parent;
  }
  return depth;
}
