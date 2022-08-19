#include "AlphaTree.h"
#include <iostream>
#include <opencv2/opencv.hpp>

using cv::Point;
using cv::Mat;

void AlphaTree::buildTree(){
	cout << "Building alpha tree:\n";
	cout << "\tCreating leaf nodes\n";
	makeLeafNodes();
	cout << "\tCreating Edges\n";
	makeEdges();
	cout << "\tBuilding alpha tree\n";
	processEdges();
	cout << "\tPruning alpha tree\n";
	int nodesPruned = pruneTree();
	cout << "\t" << nodesPruned << " nodes pruned\n";
	cout << "Finished building alpha tree.\n";
}

void AlphaTree::makeLeafNodes(){
	for(int i = 0; i < imgsize; i++){ 
		nodes[i].parent = BOTTOM;
		sets[i] = BOTTOM;
		nodes[i].alpha = LEAF_ALPHA;
		nodes[i].area = 1;
	}
	curSize = imgsize;
}

void AlphaTree::makeEdges(){
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

void AlphaTree::makeEdge(Point a, Point b){
	int a_index, b_index;
	try{
		a_index = getIndex(a, img);
		b_index = getIndex(b, img);
	} catch(PointOutOfBoundsException e){
		// One of the points was out of bounds, no need to create edge
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

void AlphaTree::processEdges(){
	while (!queue.empty()){
		Edge edge = queue.top();
		queue.pop();
		processEdge(edge);
	}
}

void AlphaTree::processEdge(Edge& edge){
	// Find the root nodes of the subtrees that the edge's pixels' nodes are in
	int root1 = findRoot(edge.p);
	int root2 = findRoot(edge.q);

	if (root1 == root2){return;} // p and q already share a subtree
	// Make sure root1 was created last, meaning it has the highest alpha value
	if (root1 < root2){std::swap(root1, root2);} 

	if (nodes[root1].alpha < edge.alpha){
		// Neither subtree goes up to the edge's alpha value
		int new_root = makeAlphaNode(edge.alpha);
		attachNodes(new_root, root1);
		attachNodes(new_root, root2);
	}
	else{
		// Attach root2 as a child to root1 
		// We already made sure root1 has a higher index and alpha value
		attachNodes(root1, root2);
	}

}

int AlphaTree::findRoot(int p){
	if(sets[p] == BOTTOM){return p;}
	int root = findRoot(sets[p]);
	sets[p] = root;
	return root;
}

int AlphaTree::makeAlphaNode(double alpha){
	// Calculate the pointer to the next free spot in the tree (pointer arithmetics)
	AlphaNode *node = nodes + curSize;
	int result = curSize;
	curSize++;
	node->alpha = alpha;
	node->parent = BOTTOM;
	sets[result] = BOTTOM;
	return result;
}

void AlphaTree::attachNodes(int parent, int child){
	nodes[child].parent = parent;
	sets[child] = parent;
	nodes[parent].area += nodes[child].area;
}

int AlphaTree::pruneTree(){
	// Indicates which nodes to keep in the pruned tree
	bool *keepNode = (bool*) calloc(curSize, sizeof(bool));
	// Indicates which indices either have a redundant node, 
	// or have been vacated by a non-redundant node.
	bool *freeIndex = (bool*) calloc(curSize, sizeof(bool));
	// For each node, indicates that node's new index in the pruned tree.
	int *nodeMap = (int*) malloc(curSize*sizeof(int));
	// Index of the next free index.
	int nextFree = -1;
	int pruneAmount = 0;

	// Iterate over nodes and prune them
	for(int i = 0; i < curSize; i++){

		// Check whether to keep or prune the node i
		if(i < imgsize){keepNode[i] = true;} //Keep all leaf nodes
		else if(!keepNode[i]){ //Node has not been claimed as parent
			if(nextFree < 0){nextFree = i;}
			freeIndex[i] = true;
			pruneAmount++;
			continue;
		}

		// Find the first non-redundant parent of node i
		int parent = nodes[i].parent;
		if (parent != BOTTOM){
			int levelParent = findLevelRoot(parent);
			nodes[i].parent = levelParent;
			keepNode[levelParent] = true;
		}

		// Map node i to its index in the pruned tree
		nodeMap[i] = i;
		if(nextFree >= 0){
			nodeMap[i] = nextFree;
			freeIndex[nextFree] = false;
			freeIndex[i] = true;
			for(; !freeIndex[nextFree] && nextFree < i; nextFree++){};
		}
	}

	free(freeIndex);

	if(nextFree < 0){ // No nodes were pruned
		free(keepNode);
		free(nodeMap);
		return 0;
	}

	// Create and populate the pruned node array
	AlphaNode *prunedNodes = (AlphaNode*) malloc((curSize - pruneAmount)*sizeof(AlphaNode));
	for(int i = 0; i < curSize; i++){
		if(!keepNode[i]){continue;}

		AlphaNode node = nodes[i];
		if(node.parent != BOTTOM){
			node.parent = nodeMap[node.parent];
		}
		prunedNodes[nodeMap[i]] = node;
	}

	free(nodes);
	free(keepNode);
	free(nodeMap);

	nodes = prunedNodes;
	curSize = curSize - pruneAmount;
	
	return pruneAmount;
}

int AlphaTree::findLevelRoot(int p){
	if(isLevelRoot(p)){return p;}
	int root = findLevelRoot(nodes[p].parent);
	nodes[p].parent = root;
	return root;
}

bool AlphaTree::isLevelRoot(int p){
	int parent = nodes[p].parent;
	if (parent == BOTTOM){return true;}
	return (nodes[p].alpha != nodes[parent].alpha);
}

int AlphaTree::size() const {
	return curSize;
}

const AlphaNode& AlphaTree::operator[] (int index) const {
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

