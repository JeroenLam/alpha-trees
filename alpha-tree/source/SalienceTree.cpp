#include "SalienceTree.h"
#include "DistanceFunction.h"
#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Mat;

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

void makeEdge(Mat img, Point a, Point b, EdgeQueue *edgeQueue, AbstractDistanceFunction *delta){
	int a_index, b_index;
	try{
		a_index = getIndex(a, img);
		b_index = getIndex(b, img);
	} catch(PointOutOfBoundsException e){
		return;
	}

	EdgeQueuePush(edgeQueue, a_index, b_index, delta->getAlpha(a, b));
}

void makeEdges(Mat img, Connectivity cn, EdgeQueue *edgeQueue, AbstractDistanceFunction *delta){
	for(int x = 0; x < img.cols; x++){
		for(int y = 0; y < img.rows; y++){
			Point p = Point(x,y);
			makeEdge(img, p, Point(x+1,y), edgeQueue, delta);
			makeEdge(img, p, Point(x,y+1), edgeQueue, delta);

			if (cn == CN_8){
				makeEdge(img, p, Point(x-1, y+1), edgeQueue, delta);
				makeEdge(img, p, Point(x+1, y+1), edgeQueue, delta);
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
int NewSalienceNode(SalienceTree *tree, int *sets, double alpha)
{
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
int FindRoot(int *sets, int p)
{
  int r = p, i, j;

  while (sets[r] != BOTTOM)
  {
    r = sets[r];
  }
  i = p;
  while (i != r)
  {
    j = sets[i];
    sets[i] = r;
    i = j;
  }
  return r;
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
boolean IsLevelRoot(SalienceTree *tree, int i)
{
  int parent = tree->nodes[i].parent;

  if (parent == BOTTOM)
    return true;
  return (tree->nodes[i].alpha != tree->nodes[parent].alpha);
}

/**
 * @brief Finds the root node of the alpha level of a given node.
 * 
 * @param tree Tree to search
 * @param p Node to find the level root of
 * @return int Index of the level root
 */
int LevelRoot(SalienceTree *tree, int p)
{
  int r = p, i, j;

  while (!IsLevelRoot(tree, r))
  {
    r = tree->nodes[r].parent;
  }
  i = p;

  while (i != r)
  {
    j = tree->nodes[i].parent;
    tree->nodes[i].parent = r;
    i = j;
  }
  return r;
}

void GetAncestors(SalienceTree *tree, int *sets, int *p, int *q)
{
  int temp;
  // get root of each pixel and ensure correct order
  *p = LevelRoot(tree, *p);
  *q = LevelRoot(tree, *q);
  if (*p < *q)
  {
    temp = *p;
    *p = *q;
    *q = temp;
  }
  // while both nodes are not the same and are not the root of the tree
  while ((*p != *q) && (sets[*p] != BOTTOM) && (sets[*q] != BOTTOM))
  {
    *q = sets[*q];
    if (*p < *q)
    {
      temp = *p;
      *p = *q;
      *q = temp;
    }
  }
  // if either node is the tree root find the root of the other
  if (sets[*p] == BOTTOM)
  {
    *q = FindRoot(sets, *q);
  }
  else if (sets[*q] == BOTTOM)
  {
    *p = FindRoot(sets, *p);
  }
}

void Union(SalienceTree *tree, int *sets, int p, int q)
{
  tree->nodes[q].parent = p;
  sets[q] = p;
  tree->nodes[p].area += tree->nodes[q].area;
}

void processEdges(SalienceTree *tree, EdgeQueue *queue, int *sets)
{
	Edge *currentEdge;
	int v1, v2, temp, r;
	double alpha12;
	while (!IsEmpty(queue))
	{
		// deque the current edge and temporarily store its values
		currentEdge = EdgeQueueFront(queue);
		v1 = currentEdge->p;
		v2 = currentEdge->q;
		GetAncestors(tree, sets, &v1, &v2);
		alpha12 = currentEdge->alpha;

		EdgeQueuePop(queue);
		if (v1 != v2)
		{
			if (v1 < v2)
			{
				temp = v1;
				v1 = v2;
				v2 = temp;
			}
			if (tree->nodes[v1].alpha < alpha12)
			{
				// if the higher node has a lower alpha level than the edge
				// we combine the two nodes in a new salience node
				r = NewSalienceNode(tree, sets, alpha12);
				Union(tree, sets, r, v1);
				Union(tree, sets, r, v2);
			}
			else
			{
				// otherwise we add the lower node to the higher node
				Union(tree, sets, v1, v2);
			}
		}
	}
}

void finalizeTree(SalienceTree *tree, int imgsize){
	bool *keepNode = (bool*) calloc(tree->curSize, sizeof(bool));
	bool *freeIndex = (bool*) calloc(tree->curSize, sizeof(bool));
	int *nodeMap = (int*) malloc(tree->curSize*sizeof(int));
	int pi = -1;
	int pruneAmount = 0;

	for(int i = 0; i < tree->curSize; i++){
		if(i < imgsize){keepNode[i] = true;} //Keep all leaf nodes
		else if(!keepNode[i]){ //Node has not been claimed as parent
			if(pi < 0){pi = i;}
			freeIndex[i] = true;
			pruneAmount++;
			continue;
		}

		int parent = tree->nodes[i].parent;
		if (parent != BOTTOM){
			int levelParent = LevelRoot(tree, parent);
			tree->nodes[i].parent = levelParent;
			keepNode[levelParent] = true;
		}

		nodeMap[i] = i;
		if(pi >= 0){
			nodeMap[i] = pi;
			freeIndex[pi] = false;
			freeIndex[i] = true;
			for(; !freeIndex[pi] && pi < i; pi++){};
		}
	}

	if(pi < 0){
		free(keepNode);
		free(nodeMap);
		return;
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
}

SalienceTree *MakeSalienceTree(Mat img, AbstractDistanceFunction *delta, Connectivity cn)
{
	int imgsize = img.cols*img.rows;

	// Priority queue holding edges between adjacent pixels, minimum edge weight has highest priority
	EdgeQueue *queue = EdgeQueueCreate((cn / 2) * imgsize);

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
	cout << "Cleaning alpha tree\n";
	finalizeTree(tree, imgsize);

	EdgeQueueDelete(queue);
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
