#include "SalienceTree.h"
#include <iostream>
#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Mat;

// i(x,y) = y*width + x
// x,y (i) = i % width, i//width

void SalienceTree::buildTree(bool prune){
	cout << "Building alpha tree:\n";
	cout << "\tCreating leaf nodes\n";
	makeLeafNodes();
	cout << "\tCreating Edges\n";
	makeEdges();
	cout << "\tBuilding alpha tree\n";
	processEdges();
	if(prune){
		cout << "\tPruning alpha tree\n";
		int nodesPruned = pruneTree();
		cout << "\t" << nodesPruned << " nodes pruned\n";
	}
	cout << "Finished building alpha tree.\n";
}

void SalienceTree::makeLeafNodes(){
	curSize = imgsize;
	for(int i = 0; i < imgsize; i++){ 
		nodes[i].parent = BOTTOM;
		sets[i] = BOTTOM;
		nodes[i].alpha = LEAF_ALPHA;
		nodes[i].area = 1;
	}
}

void SalienceTree::makeEdges(){
	for(int x = 0; x < img.cols; x++){
		for(int y = 0; y < img.rows; y++){
			Point p = Point(x,y);

			makeEdge(p, Point(x+1,y));
			makeEdge(p, Point(x, y+1));

			if (cn == CN_8){
				makeEdge(p, Point(x-1, y+1));
				makeEdge(p, Point(x+1, y+1));
			}
		}
	}
}

void SalienceTree::makeEdge(Point a, Point b){
	int a_index, b_index;
	try{
		a_index = getIndex(a, img);
		b_index = getIndex(b, img);
	} catch(PointOutOfBoundsException e){
		return;
	}
	
	Edge e = {.p = a_index, .q = b_index, .alpha = delta.getAlpha(a,b)}; 
	if(e.alpha <= lambdamin){
		e.alpha = lambdamin;
		processEdge(e);
	}else if(!excludeTop){
		queue.push(e);
	}
}

void SalienceTree::processEdges(){
	while (!queue.empty()){
		// deque the current edge and temporarily store its values
		Edge edge = queue.top();
		queue.pop();
		processEdge(edge);
	}
}

void SalienceTree::processEdge(Edge& edge){
	int root1 = findRoot(edge.p);
	int root2 = findRoot(edge.q);

	if (root1 == root2){return;}
	if (root1 < root2){std::swap(root1, root2);}

	if (nodes[root1].alpha < edge.alpha){
		// if the higher node has a lower alpha level than the edge
		// we combine the two nodes in a new salience node
		int new_root = makeSalienceNode(edge.alpha);
		mergeNodes(new_root, root1);
		mergeNodes(new_root, root2);
	}
	else{
		// otherwise we add the lower node to the higher node
		mergeNodes(root1, root2);
	}

}

/**
 * @brief Finds the root of a set of nodes. Also performs path compression.
 *
 * @param sets The array containing the set graph
 * @param p the index in the sets array of the node whose set's root needs to be found
 */
int SalienceTree::findRoot(int p){
	if(sets[p] == BOTTOM){return p;}
	int root = findRoot(sets[p]);
	sets[p] = root;
	return root;
}

/**
 * @brief Create a Salience Node object in a given tree
 * 
 * @param tree Tree to which the node will be added
 * @param sets 
 * @param alpha The nodes alpha level
 * @return int Index of the new node
 */
int SalienceTree::makeSalienceNode(double alpha){
	// node is the next free spot in the tree (pointer arithmetics)
	SalienceNode *node = nodes + curSize;
	int result = curSize;
	curSize++;
	node->alpha = alpha;
	node->parent = BOTTOM;
	sets[result] = BOTTOM;
	return result;
}

void SalienceTree::mergeNodes(int p, int q){
	nodes[q].parent = p;
	sets[q] = p;
	nodes[p].area += nodes[q].area;
}

int SalienceTree::pruneTree(){
	bool *keepNode = (bool*) calloc(curSize, sizeof(bool));
	bool *freeIndex = (bool*) calloc(curSize, sizeof(bool));
	int *nodeMap = (int*) malloc(curSize*sizeof(int));
	int nextFree = -1;
	int pruneAmount = 0;

	for(int i = 0; i < curSize; i++){
		if(i < imgsize){keepNode[i] = true;} //Keep all leaf nodes
		else if(!keepNode[i]){ //Node has not been claimed as parent
			if(nextFree < 0){nextFree = i;}
			freeIndex[i] = true;
			pruneAmount++;
			continue;
		}

		int parent = nodes[i].parent;
		if (parent != BOTTOM){
			int levelParent = findLevelRoot(parent);
			nodes[i].parent = levelParent;
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

	SalienceNode *prunedNodes = (SalienceNode*) malloc((curSize - pruneAmount)*sizeof(SalienceNode));
	for(int i = 0; i < curSize; i++){
		if(!keepNode[i]){continue;}

		SalienceNode node = nodes[i];
		if(node.parent != BOTTOM){
			node.parent = nodeMap[node.parent];
		}
		prunedNodes[nodeMap[i]] = node;
		
	}
	free(nodes);
	nodes = prunedNodes;
	curSize = curSize - pruneAmount;

	free(keepNode);
	free(nodeMap);
	
	return pruneAmount;
}

/**
 * @brief Finds the root node of the alpha level of a given node.
 * 
 * @param tree Tree to search
 * @param p Node to find the level root of
 * @return int Index of the level root
 */
int SalienceTree::findLevelRoot(int p){
	if(isLevelRoot(p)){return p;}
	int root = findLevelRoot(nodes[p].parent);
	nodes[p].parent = root;
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
bool SalienceTree::isLevelRoot(int i){
	int parent = nodes[i].parent;
	if (parent == BOTTOM){return true;}
	return (nodes[i].alpha != nodes[parent].alpha);
}

int SalienceTree::size() const {
	return curSize;
}

const SalienceNode& SalienceTree::operator[] (int index) const {
	return nodes[index];
}

Point getPoint(int index, const Mat& img){
	int x = index % img.cols;
	int y = index / img.cols;
	if(y < 0 || x < 0 || y >= img.rows || x >= img.cols)
		throw PointOutOfBoundsException();
	return Point(x,y);
}

int getIndex(Point& p, const Mat& img){
	if(p.y < 0 || p.x < 0 || p.y >= img.rows || p.x >= img.cols)
		throw PointOutOfBoundsException();
	return p.y*img.cols + p.x;
}

