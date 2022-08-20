#ifndef ALPHA_TREE_H
#define ALPHA_TREE_H

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

/**
 * Connectivity of the graph representation of an image.
 *
 * Each pixel can either have an edge connected to four or eight of its
 * neighbours.
 */
enum Connectivity {CN_4 = 4, CN_8 = 8};

/**
 * Represents an edge between two neighbouring pixels.
 */
struct Edge{
	// Indices of the leaf nodes in the tree connected by this edge.
	int p, q;
	// Alpha value of the edge. I.e. distance between the values of pixels p and q.
	double alpha;

	/**
	 * Implements ordering such that lhs > rhs if lhs.alpha <= rhs.alpha,
	 * I.e. if lhs has a higher priority.
	 */
	friend bool operator< (const Edge& lhs, const Edge& rhs){
		return lhs.alpha >= rhs.alpha;
	}
};

/**
 * Derivation of a priority_queue for Edge objects which pre-reserves the
 * required size in order to prevent container reallocation.
 */
class EdgeQueue : public priority_queue<Edge, vector<Edge>>{
	public:
		EdgeQueue(int size){
			this->c.reserve(size);
		}
};

/**
 * Represents a node in an AlphaTree.
 */
struct AlphaNode{
	// Index of this node's parent node
	int parent;
	// Number of pixels/leaf nodes in this node's subtree
	int area;
	// Alpha value of this node
	double alpha;  
};

/**
 * A basic alpha tree implementation.
 * 
 * The tree contains an array of AlphaNode objects which can be read using the
 * [] operator. The following are given:
 * 	- Nodes come before their parent in the array
 * 	- A node's alpha value is always lower than its parent's
 * 	- Every leaf node represents a pixel
 * 	- The array contains the exact number of nodes as returned by AlphaTree::size()
 */
class AlphaTree{
	private:		
		/*-----------------------------------------------------------*/
		/* Constructor arguments ------------------------------------*/

		// The image that the alpha tree is based on
		const Mat& img;
		// Distance function used to calculate the alpa value of the edge between two pixels
		const AbstractDistanceFunction& delta;
		// Connectivity of the the image
		const Connectivity cn;
		// Minimum alpha level that a non-leaf node in the tree should have
		const double lambdamin;
		// Whether or not nodes with a higher alpha level than lambdamin should be excluded
		const bool excludeTop;

		/*-----------------------------------------------------------*/
		/* Size-related variables -----------------------------------*/

		// Current amount of nodes in the tree
		int curSize = 0;
		// Total number of pixels in the image
		const int imgsize;

		/*-----------------------------------------------------------*/
		/* Data structures ------------------------------------------*/

		/** 
		 * Priority queue of Edge objects used to make sure edges are 
		 * processed with ascending alpha values.
		 */
		EdgeQueue queue;

		/** 
		 * The array of AlphaNode objects that the tree consists of.
		 * At first, the array only contains leaf nodes representing the
		 * image's pixels. For each edge coming out of the edge queue,
		 * the subtrees containing the leaf nodes corresponding to the 
		 * pixels connected by the edge are merged into one subtree.
		 */
		AlphaNode *nodes;

		/** 
		 * Array used for the Union-Find algorithm, which is used to 
		 * quickly find the root node of a subtree during construction.
		 *
		 * (sets[p] == q) means that p and q are in the same set, i.e.
		 * the same subtree.
		 * (sets[p] == BOTTOM) means that p is the root/representative
		 * element of its set.
		 */
		int *sets;

		/*-----------------------------------------------------------*/
		/* Operations used to construct the tree---------------------*/

		/**
		 * Create an edge between two points in the image. 
		 *
		 * If the edge's alpha value is less than or equal to 
		 * lambdamin, its alpha value is set to lambdamin and it is 
		 * immediately processed.
		 *
		 * If the edge's alpha value is higher than lambdamin and 
		 * excludeTop is set to true, the edge is discarded.
		 *
		 * Otherwise, it is enqueued in the edge queue
		 *
		 * If either of the points lies outside the image, no edge is 
		 * created.
		 *
		 * @param a, b Point in 2D space corresponding to a pixel
		 */
		void makeEdge(Point a, Point b);
		
		/**
		 * Processes a single Edge by making sure the subtrees 
		 * containing the edge's pixels are connected by a node with 
		 * an alpha value less than or equal to the edge's alpha value.
		 * 
		 * @param edge The edge whose pixel's subtrees should be connected
		 */
		void processEdge(Edge &edge);

		/**
		 * Creates a new node with default values at the end of the
		 * nodes array.
		 *
		 * @param alpha The alpha value of the new node.
		 * @return the index of the newly made node
		 */
		int makeAlphaNode(double alpha);

		/**
		 * Attaches a node as a child to another.
		 *
		 * @param parent index of the node to which the child is to be attached
		 * @param child index of the node which is to be attached to the parent
		 */
		void attachNodes(int parent, int child);
		
		/*-----------------------------------------------------------*/
		/* Query methods used while constructing the tree------------*/

		/**
		 * Finds the root of the subtree of a node. Also performs path
		 * compression on each element of the sets array that was 
		 * traversed.
		 *
		 * @param p the index of the node whose subtree's root should be returned
		 * @return the index of `p`'s subtree's root node
		 */
		int findRoot(int p);

		/**
		 * Finds the highest node in the tree that the given node is a
		 * descendant of, which has the same alpha level as that node.
		 *
		 * Due to the fact that edges are considered one by one, 
		 * sometimes subtrees whose roots have identical alpha values 
		 * are merged. The root node that did not become the root of 
		 * the merged subtree is then redundant, since it defines the 
		 * same tier in the alpha tree as its parent. 
		 *
		 * This function goes up the tree starting at the given node,
		 * until the first non-redundant node is found, which is 
		 * returned. Also attaches each traversed node to the found 
		 * non-redundant node in order to reduce unecessary tree depth.
		 *
		 * @param p index of the node at which to start
		 * @return either p, or the index of a non-redundant node above p with the same alpha level
		 */
		int findLevelRoot(int p);

		/**
		 * Whether the given node is non-redundant at the moment the
		 * function is called.
		 *
		 * @param p Index of a node in the tree
		 * @return whether the given node is non-redundant
		 */
		bool isLevelRoot(int p);

		/*-----------------------------------------------------------*/
		/* Methods representing the phases of tree construction------*/

		/**
		 * Creates the appropriate edges for each pixel in the image,
		 * and enqueues them if necessary.
		 */
		void makeEdges();

		/**
		 * Creates a leaf node for every pixel in the image. All leaf
		 * nodes have an index less than imgsize and can be mapped back
		 * to the location of their corresponding pixel using the 
		 * getPoint function
		 */
		void makeLeafNodes();

		/**
		 * Processes all enqueued edges by merging the subtrees of the 
		 * pixels of each edge.
		 */
		void processEdges();

		/**
		 * Removes redundant nodes from the tree and compresses the
		 * remaining nodes to be contiguous. Also reallocates the nodes
		 * array to be the same size as the number of remaining nodes.
		 *
		 * @return The number of redundant nodes that were removed from the tree.
		 */
		int pruneTree();

		/**
		 * Calls each of the stages above in order.
		 */
		void buildTree();

	public:
		
		/**
		 * @param img Image used to build the tree.
		 * @param delta Distance function used to calculate the alpha level of edges between pixels
		 * @param cn Connectivity of the image's pixels
		 * @param lambdamin Minimum alpha value a non-leaf node in the tree should have
		 * @param excludeTop Whether nodes with a value higher than lambdamin should be ignored. If true, the tree only defines lambdamin-flatzones.
		 */
		AlphaTree(const Mat& img, const AbstractDistanceFunction& delta, Connectivity cn, double lambdamin = 0, double excludeTop = false)
		: img(img)
		, delta(delta)
		, cn(cn)
		, lambdamin(lambdamin)
		, excludeTop(excludeTop)
		, imgsize(img.cols*img.rows)
		, queue((cn/2)*img.cols*img.rows){

			sets = (int*) malloc(imgsize*2*sizeof(int));
			nodes = (AlphaNode*) malloc((2*imgsize) * sizeof(AlphaNode));
			buildTree();
			free(sets);
		};
		
		~AlphaTree(){
			free(nodes);
		}

		/**
		 * @return number of nodes in the tree
		 */
		int size() const;

		/**
		 * @return The AlphaNode with the given index
		 */
		const AlphaNode& operator[] (int index) const;
};

/**
 * Exception thrown by getPoint when the given index does not correspond to a 
 * leaf/pixel node, and by getIndex if the given point lies outside the given image.
 */
class PointOutOfBoundsException : std::exception {
	public:
		std::string what (){ return "Point out of bounds"; }

};

/**
 * Returns the 2D point corresponding to the leaf/pixel node indexed by the
 * given index.
 *
 * @param index The index of the node whose point in the image should be found
 * @param image The image matrix that the node's pixel is in.
 * @throws PointOutOfBoundsException if the given index does not correspond to a leaf/pixel node.
 * @return the 2D point in the given image where the pixel identified by the given index is located.
 */
Point getPoint(int index, const Mat& image);

/**
 * Returns the index of the node corresponding to the pixel in the given image
 * that corresponds to the given 2D point.
 *
 * @param p Any 2D point.
 * @param image The image that the node's pixel is in.
 * @throws PointOutOfBoundsException if the given point lies outside the given image.
 * @return the index of the node corresponding to the pixel identified by the given point.
 */
int getIndex(Point& p, const Mat& image);

#endif
