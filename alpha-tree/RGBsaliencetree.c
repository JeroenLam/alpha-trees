#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <assert.h>

#define BOTTOM (-1)
#define false 0
#define true 1
#define MIN(a, b) ((a <= b) ? (a) : (b))
#define MAX(a, b) ((a >= b) ? (a) : (b))

#define CONNECTIVITY 4

typedef short bool;
typedef unsigned char ubyte;
double RGBweight[3] = {0.50, 0.5, 0.5};

double MainEdgeWeight = 1.0;
double OrthogonalEdgeWeight = 1.0;

int width, height, size;

int lambda;
double omegafactor = 200000;

// ubyte is an 8-bit unsigned integral data type range [0,255]
// => Pixel is an array of 3 colors in range [0,255]
typedef ubyte Pixel[3];

// input and output images as arrays of pixel
Pixel *gval = NULL, *out = NULL;

// Edge representation where p, q are indices of two pixels and alpha is the aplha value between them
typedef struct Edge
{
  int p, q;
  double alpha;
} Edge;

// queue of edges
typedef struct
{
  int size, maxsize;
  Edge *queue;
} EdgeQueue;

/**
 * @brief Allocates space for a new EdgeQueue and initializes its values.
 * 
 * @param maxsize Maximum amount of Edges the queue can hold
 * @return EdgeQueue* The created queue
 */
EdgeQueue *EdgeQueueCreate(long maxsize)
{
  EdgeQueue *newQueue = (EdgeQueue *)malloc(sizeof(EdgeQueue));
  newQueue->size = 0;
  newQueue->queue = (Edge *)malloc((maxsize + 1) * sizeof(Edge));
  newQueue->maxsize = maxsize;
  return newQueue;
}

#define EdgeQueueFront(queue) (queue->queue + 1)
#define IsEmpty(queue) ((queue->size) == 0)

/**
 * @brief Free the allocated memory of an EdgeQueue
 * 
 * @param oldqueue The queue to free the memory of
 */
void EdgeQueueDelete(EdgeQueue *oldqueue)
{
  free(oldqueue->queue);
  free(oldqueue);
}

void EdgeQueuePop(EdgeQueue *queue)
{
  int current = 1;
  Edge moved;
  // we want to pop the edge at the end of the queue
  moved.p = queue->queue[queue->size].p;
  moved.q = queue->queue[queue->size].q;
  moved.alpha = queue->queue[queue->size].alpha;

  queue->size--;

  // while one of the edges children has a lower alpha value than the popped edge
  while (((current * 2 <= queue->size) &&
          (moved.alpha > queue->queue[current * 2].alpha)) ||
         ((current * 2 + 1 <= queue->size) &&
          (moved.alpha > queue->queue[current * 2 + 1].alpha)))
  {
    // right child is the lower alpha
    if ((current * 2 + 1 <= queue->size) &&
        (queue->queue[current * 2].alpha >
         queue->queue[current * 2 + 1].alpha))
    {
      queue->queue[current].p = queue->queue[current * 2 + 1].p;
      queue->queue[current].q = queue->queue[current * 2 + 1].q;
      queue->queue[current].alpha = queue->queue[current * 2 + 1].alpha;
      current += current + 1;
    }
    // left child is the lower alpha
    else
    {
      queue->queue[current].p = queue->queue[current * 2].p;
      queue->queue[current].q = queue->queue[current * 2].q;
      queue->queue[current].alpha = queue->queue[current * 2].alpha;
      current += current;
    }
  }
  queue->queue[current].p = moved.p;
  queue->queue[current].q = moved.q;
  queue->queue[current].alpha = moved.alpha;
}

/**
 * @brief Inserts an edge defined by its values into a given EdgeQueue.
 * The edge that is added is inserted into the queue so that all its children have 
 * larger alpha values.
 * 
 * @param queue EdgeQueue into which to add the edge
 * @param p 
 * @param q 
 * @param alpha Alphs value of the edge
 */
void EdgeQueuePush(EdgeQueue *queue, int p, int q, double alpha)
{
  long current;
  
  // increase the amount of elements in the queue and update where
  // the queue points to
  queue->size++;
  current = queue->size;

  // while we do not look at the root and the parents alpha is higer than the given alpha
  while ((current / 2 != 0) && (queue->queue[current / 2].alpha > alpha))
  {
    // swap the parent to the current node
    queue->queue[current].p = queue->queue[current / 2].p;
    queue->queue[current].q = queue->queue[current / 2].q;
    queue->queue[current].alpha = queue->queue[current / 2].alpha;
    current = current / 2;
  }
  // set lastly swapped parent to the given value
  queue->queue[current].p = p;
  queue->queue[current].q = q;
  queue->queue[current].alpha = alpha;
}

typedef struct SalienceNode
{
  int parent;
  int area;
  bool filtered; /* indicates whether or not the filtered value is OK */
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
  SalienceNode *node;
} SalienceTree;

/**
 * @brief Create a Salience Tree object
 * 
 * @param imgsize Size of the image
 * @return SalienceTree* Newly created Salience Tree
 */
SalienceTree *CreateSalienceTree(int imgsize)
{
  SalienceTree *tree = malloc(sizeof(SalienceTree));
  tree->maxSize = 2 * imgsize; /* potentially twice the number of nodes as pixels exist*/
  tree->curSize = imgsize;     /* first imgsize taken up by pixels */
  tree->node = malloc((tree->maxSize) * sizeof(SalienceNode));
  return tree;
}

/**
 * @brief Free memory allocated for a given Salience Tree
 * 
 * @param tree Tree to free the memory of
 */
void DeleteTree(SalienceTree *tree)
{
  free(tree->node);
  free(tree);
}

/**
 * @brief Create a Salience Node object
 * 
 * @param tree Tree to which the node will be added
 * @param root 
 * @param alpha The nodes alpha level
 * @return int Index of the new node
 */
int NewSalienceNode(SalienceTree *tree, int *root, double alpha)
{
  // node is the next free spot in the tree (pointer arithmetics)
  SalienceNode *node = tree->node + tree->curSize;
  int result = tree->curSize;
  tree->curSize++;
  node->alpha = alpha;
  node->parent = BOTTOM;
  root[result] = BOTTOM;
  return result;
}

/**
 * @brief Initializes a given node in the AlphaTree so that it can be used
 * in the algorithm.
 * 
 * @param tree Tree of the node
 * @param root 
 * @param gval Array of pixels in the original image
 * @param p Index of the node in the tree
 */
void MakeSet(SalienceTree *tree, int *root, Pixel *gval, int p)
{
  int i;
  tree->node[p].parent = BOTTOM;
  root[p] = BOTTOM;
  tree->node[p].alpha = 0.0;
  tree->node[p].area = 1;
  for (i = 0; i < 3; i++)
  {
    tree->node[p].sumPix[i] = gval[p][i];
    tree->node[p].minPix[i] = gval[p][i];
    tree->node[p].maxPix[i] = gval[p][i];
  }
}

int FindRoot(int *root, int p)
{
  int r = p, i, j;

  while (root[r] != BOTTOM)
  {
    r = root[r];
  }
  i = p;
  while (i != r)
  {
    j = root[i];
    root[i] = r;
    i = j;
  }
  return r;
}

int FindRoot1(SalienceTree *tree, int *root, int p)
{
  int r, i, j;
  r = p;

  // make r the root of the tree
  while (root[r] != BOTTOM)
  {
    r = root[r];
  }
  i = p;
  /*
   * r = ROOT
   * i = current Pixel
   * invariant: current Pixel != ROOT
   */
  while (i != r)
  {
    j = root[i];
    // i's root becomes the total root
    root[i] = r;
    // also change the parent in the tree to the total root
    tree->node[i].parent = r;
    // i becomes its own root
    i = j;
  }
  return r;
}

/**
 * @brief Computes the salience between two pixels using an unweighted average.
 * computes sqrt(sum((P1 - P2)^2))
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double simpleSalience(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  return sqrt(result);
}

/**
 * @brief Computes the salience between two pixels using a weighted average.
 * computes sqrt(sum(W*(P1 - P2)^2))
 * 
 * @param p First Pixel
 * @param q Second Pixel
 * @return double result of the computation
 */
double WeightedSalience(Pixel p, Pixel q)
{
  double result = 0;
  int i;

  for (i = 0; i < 3; i++)
    result += RGBweight[i] * ((double)p[i] - (double)q[i]) * ((double)p[i] - (double)q[i]);
  return sqrt(result);
}

/**
 * @brief Computes the edge strength in the x direction at a given position (x,y)
 * 
 * @param img Image to compute in
 * @param width of the image
 * @param height of the image
 * @param x x-coordinate of the position
 * @param y y-coordinate of the position
 * @return double The edge strength
 */
double EdgeStrengthX(Pixel *img, int width, int height, int x, int y)
{
  int yminus1 = y - (y > 0);
  int yplus1 = y + (y < height - 1);

  // We use the minimum salience between the sourrounding rows at (x-1) and x
  double ygrad = MIN(
    WeightedSalience(
      img[width * yminus1 + x - 1],
      img[width * yplus1 + x - 1]
    ),
    WeightedSalience(
      img[width * yminus1 + x],
      img[width * yplus1 + x]
    )
  );
  return (
    OrthogonalEdgeWeight * 
    ygrad + 
    MainEdgeWeight *
    WeightedSalience(
      img[width * y + x - 1],
      img[width * y + x]
    )
  );
}

/**
 * @brief Computes the edge strength in the y direction at a given position (x,y)
 * 
 * @param img Image to compute in
 * @param width of the image
 * @param height of the image
 * @param x x-coordinate of the position
 * @param y y-coordinate of the position
 * @return double The edge strength
 */
double EdgeStrengthY(Pixel *img, int width, int height, int x, int y)
{
  int xminus1 = x - (x > 0);
  int xplus1 = x + (x < width - 1);

  // We use the minimum salience between the sourrounding columns at (y-1) and y
  double xgrad = MIN(
    WeightedSalience(
      img[width * y + xplus1],
      img[width * y + xminus1]
    ),
    WeightedSalience(
      img[width * (y - 1) + xplus1],
      img[width * (y - 1) + xminus1]
    )
  );
  return (
    OrthogonalEdgeWeight * 
    xgrad + 
    MainEdgeWeight *
    WeightedSalience(
      img[width * (y - 1) + x],
      img[width * y + x]
    )
  );
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
bool IsLevelRoot(SalienceTree *tree, int i)
{
  int parent = tree->node[i].parent;

  if (parent == BOTTOM)
    return true;
  return (tree->node[i].alpha != tree->node[parent].alpha);
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
    r = tree->node[r].parent;
  }
  i = p;

  while (i != r)
  {
    j = tree->node[i].parent;
    tree->node[i].parent = r;
    i = j;
  }
  return r;
}

#define Par(tree, p) LevelRoot(tree, tree->node[p].parent)

/**
 * @brief Combines the regions of two pixels.
 * 
 * @param tree Tree to work on
 * @param root 
 * @param p First Pixel
 * @param q Second Pixel
 */
void Union(SalienceTree *tree, int *root, int p, int q)
{ /* p is always current pixel */
  int i;

  q = FindRoot1(tree, root, q);

  // if q's parent is not p
  if (q != p)
  {
    // set p to be q's parent
    tree->node[q].parent = p;
    root[q] = p;
    // increase the area as p now has more pixels as children
    tree->node[p].area += tree->node[q].area;
    // update total pixel sum, minimum pixel and maximum pixel values
    for (i = 0; i < 3; i++)
    {
      tree->node[p].sumPix[i] += tree->node[q].sumPix[i];
      tree->node[p].minPix[i] = MIN(tree->node[p].minPix[i], tree->node[q].minPix[i]);
      tree->node[p].maxPix[i] = MAX(tree->node[p].maxPix[i], tree->node[q].maxPix[i]);
    }
  }
}

void Union2(SalienceTree *tree, int *root, int p, int q)
{
  int i;
  tree->node[q].parent = p;
  root[q] = p;
  tree->node[p].area += tree->node[q].area;
  for (i = 0; i < 3; i++)
  {
    tree->node[p].sumPix[i] += tree->node[q].sumPix[i];
    tree->node[p].minPix[i] = MIN(tree->node[p].minPix[i], tree->node[q].minPix[i]);
    tree->node[p].maxPix[i] = MAX(tree->node[p].maxPix[i], tree->node[q].maxPix[i]);
  }
}

/**
 * @brief Assesses the whole image once. Each pixel is investigated and the
 * edge strength between it and its defined neighbors is evaluated. Based on
 * this edge strength pixels are either combined in the salience tree or
 * they are stored as edges in the edge queue.
 * 
 * @param tree Salience Tree we are working on
 * @param queue Edge queue to push to
 * @param root 
 * @param img Image we are working on
 * @param width of the image
 * @param height of the image
 * @param lambdamin threshold to determine if we have encountered an edge
 */
void Phase1(SalienceTree *tree, EdgeQueue *queue, int *root, Pixel *img, int width, int height, double lambdamin)
{
  // TODO make the EdgeStrength functions replaceable so that we can use different dissimilarity measures
  /* pre: tree has been created with imgsize= width*height
          queue initialized accordingly;
   */
  int imgsize = width * height;
  int p, x, y;
  double edgeSalience;

  // root is a separate case
  MakeSet(tree, root, img, 0);

  // for the first row in the image
  for (x = 1; x < width; x++)
  {
    // ready current node and find edge strength of the current position
    MakeSet(tree, root, img, x);
    edgeSalience = EdgeStrengthX(img, width, height, x, 0);
    if (edgeSalience < lambdamin)
    {
      // if we evaluate as no edge then we combine the current and last pixel
      Union(tree, root, x, x - 1);
    }
    else
    {
      // otherwise we store the found edge
      EdgeQueuePush(queue, x, x - 1, edgeSalience);
    }
  }

  // for all other rows
  for (y = 1; y < height; y++)
  {
    // p is the first pixel in the row
    p = y * width;
    // ready current node and find edge strength of the current position
    MakeSet(tree, root, img, p);
    edgeSalience = EdgeStrengthY(img, width, height, 0, y);

    if (edgeSalience < lambdamin)
    {
      // if we evaluate as no edge then we combine the current and last pixel
      Union(tree, root, p, p - width);
    }
    else
    {
      // otherwise we store the found edge
      EdgeQueuePush(queue, p, p - width, edgeSalience);
    }
    p++;
    // for each column in the current row
    for (x = 1; x < width; x++, p++)
    {
      // reapeat process in y-direction
      MakeSet(tree, root, img, p);
      edgeSalience = EdgeStrengthY(img, width, height, x, y);
      if (edgeSalience < lambdamin)
      {
        Union(tree, root, p, p - width);
      }
      else
      {
        EdgeQueuePush(queue, p, p - width, edgeSalience);
      }
      // repeat process in x-direction
      edgeSalience = EdgeStrengthX(img, width, height, x, y);
      if (edgeSalience < lambdamin)
      {
        Union(tree, root, p, p - 1);
      }
      else
      {
        EdgeQueuePush(queue, p, p - 1, edgeSalience);
      }
    }
  }
}

void GetAncestors(SalienceTree *tree, int *root, int *p, int *q)
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
  while ((*p != *q) && (root[*p] != BOTTOM) && (root[*q] != BOTTOM))
  {
    *q = root[*q];
    if (*p < *q)
    {
      temp = *p;
      *p = *q;
      *q = temp;
    }
  }
  // if either node is the tree root find the root of the other
  if (root[*p] == BOTTOM)
  {
    *q = FindRoot(root, *q);
  }
  else if (root[*q] == BOTTOM)
  {
    *p = FindRoot(root, *p);
  }
}

void Phase2(SalienceTree *tree, EdgeQueue *queue, int *root, Pixel *img, int width, int height)
{
  Edge *currentEdge;
  int v1, v2, temp, r;
  double oldalpha, alpha12;
  oldalpha = 0;
  while (!IsEmpty(queue))
  {
    // deque the current edge and temporarily store its values
    currentEdge = EdgeQueueFront(queue);
    v1 = currentEdge->p;
    v2 = currentEdge->q;
    GetAncestors(tree, root, &v1, &v2);
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
      if (tree->node[v1].alpha < alpha12)
      {
        // if the higher node has a lower alpha level than the edge
        // we combine the two nodes in a new salience node
        r = NewSalienceNode(tree, root, alpha12);
        Union2(tree, root, r, v1);
        Union2(tree, root, r, v2);
      }
      else
      {
        // otherwise we add the lower node to the higher node
        Union2(tree, root, v1, v2);
      }
    }
    // store last edge alpha
    oldalpha = alpha12;
  }
}

SalienceTree *MakeSalienceTree(Pixel *img, int width, int height, double lambdamin)
{
  int imgsize = width * height;
  EdgeQueue *queue = EdgeQueueCreate((CONNECTIVITY / 2) * imgsize);
  // TODO what does the root array represent?
  int *root = malloc(imgsize * 2 * sizeof(int));
  SalienceTree *tree;
  tree = CreateSalienceTree(imgsize);
  assert(tree != NULL);
  assert(tree->node != NULL);
  fprintf(stderr, "Phase1 started\n");
  // Phase 1 combines nodes that are not seen as edges and fills the edge queue with found edges
  Phase1(tree, queue, root, img, width, height, lambdamin);
  fprintf(stderr, "Phase2 started\n");
  // Phase 2 runs over all edges, creates SalienceNodes and 
  Phase2(tree, queue, root, img, width, height);
  fprintf(stderr, "Phase2 done\n");
  EdgeQueueDelete(queue);
  free(root);
  return tree;
}

/**
 * @brief Sets the color of the out image. The color is set as the average of
 * the pixels contained in its alpha level. The determining factor in this filter
 * is the area of an alpha level. It is constantly compared to the lambda level.
 * 
 * @param tree Tree to draw
 * @param out Out image
 * @param lambda user defined parameter
 */
void SalienceTreeAreaFilter(SalienceTree *tree, Pixel *out, int lambda)
{
  int i, j, imgsize = tree->maxSize / 2;
  if (lambda <= imgsize)
  {
    // set the outval of the last node
    for (j = 0; j < 3; j++)
    {
      tree->node[tree->curSize - 1].outval[j] =
          tree->node[tree->curSize - 1].sumPix[j] / tree->node[tree->curSize - 1].area;
    }
    // set color of all other nodes
    for (i = tree->curSize - 2; i >= 0; i--)
    {
      // check if we are dealing with the level root and if it has the right area
      if (IsLevelRoot(tree, i) && (tree->node[i].area >= lambda))
      {
        // set the color of the level root
        for (j = 0; j < 3; j++)
          tree->node[i].outval[j] = tree->node[i].sumPix[j] / tree->node[i].area;
      }
      else
      {
        // use the parents color
        for (j = 0; j < 3; j++)
          tree->node[i].outval[j] = tree->node[tree->node[i].parent].outval[j];
      }
    }
  }
  else
  {
    // if lambda is larger than the image size we get a black image
    for (i = tree->curSize - 1; i >= 0; i--)
    {
      for (j = 0; j < 3; j++)
        tree->node[i].outval[j] = 0;
    }
  }
  // set colors of the out image
  for (i = 0; i < imgsize; i++)
    for (j = 0; j < 3; j++)
      out[i][j] = tree->node[i].outval[j];
}

#define NodeSalience(tree, p) (tree->node[Par(tree, p)].alpha)

/**
 * @brief Sets the color of the out image. The color is set as the average of
 * the pixels contained in its alpha level. The determining factor in this filter
 * is the salience of the nodes. It is constantly compared to the lambda level.
 * The node salience is the alpha level of its parent.
 * 
 * @param tree Tree to draw
 * @param out Out image
 * @param lambda user defined parameter
 */
void SalienceTreeSalienceFilter(SalienceTree *tree, Pixel *out, double lambda)
{
  int i, j, imgsize = tree->maxSize / 2;
  if (lambda <= tree->node[tree->curSize - 1].alpha)
  {
    // set the outval of the last node
    for (j = 0; j < 3; j++)
    {
      tree->node[tree->curSize - 1].outval[j] =
          tree->node[tree->curSize - 1].sumPix[j] / tree->node[tree->curSize - 1].area;
    }
    // set color of all other nodes
    for (i = tree->curSize - 2; i >= 0; i--)
    {
      // check if we are dealing with the level root and if it has the right salience
      if (IsLevelRoot(tree, i) && (NodeSalience(tree, i) >= lambda))
      {
        // set the color of the level root
        for (j = 0; j < 3; j++)
          tree->node[i].outval[j] = tree->node[i].sumPix[j] / tree->node[i].area;
      }
      else
      {
        // use parents color
        for (j = 0; j < 3; j++)
          tree->node[i].outval[j] = tree->node[tree->node[i].parent].outval[j];
      }
    }
  }
  else
  {
    // if lambda is larger than the root alpha we get a black image
    for (i = tree->curSize - 1; i >= 0; i--)
    {
      for (j = 0; j < 3; j++)
        tree->node[i].outval[j] = 0;
    }
  }
  // set colors of the out image
  for (i = 0; i < imgsize; i++)
    for (j = 0; j < 3; j++)
      out[i][j] = tree->node[i].outval[j];
}

/**
 * @brief Reads contents of a given ppm image into the global gval Pixel array.
 * The ppm image should be encoded in ASCII format. It contains the P3 ppm header signature.
 *
 * @param fname Path to the ppm image to read
 * @return short 0 on failure, 1 otherwise
 */
short ImagePPMAsciiRead(char *fname)
{
  FILE *infile;
  unsigned long i, j;
  int c;

  infile = fopen(fname, "r");
  if (infile == NULL)
  {
    fprintf(stderr, "Error: Can't read the ASCII file: %s !", fname);
    return (0);
  }
  fscanf(infile, "P3\n");
  // skip comments
  while ((c = fgetc(infile)) == '#')
    while ((c = fgetc(infile)) != '\n')
      ;
  ungetc(c, infile);

  // scan width and height from ppm and assume color spectrum [0,255]
  fscanf(infile, "%d %d\n255\n", &width, &height);
  size = width * height;

  // allocate space for all pixels in the image
  gval = malloc(size * sizeof(Pixel));
  if (gval == NULL)
  {
    fprintf(stderr, "Out of memory!");
    fclose(infile);
    return (0);
  }
  // read in all pixels from ppm image
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < 3; j++)
    {
      fscanf(infile, "%d", &c);
      gval[i][j] = c;
    }
  }
  fclose(infile);
  return (1);
} /* ImagePGMAsciiRead */

/**
 * @brief Reads contents of a given ppm image into the global gval Pixel array.
 * The ppm image should be encoded in Binary format. It contains the P6 ppm header signature.
 *
 * @param fname
 * @return short
 */
short ImagePPMBinRead(char *fname)
{
  FILE *infile;
  int c, i;

  infile = fopen(fname, "rb");
  if (infile == NULL)
  {
    fprintf(stderr, "Error: Can't read the binary file: %s !", fname);
    return (0);
  }
  fscanf(infile, "P6\n");
  // skip comments
  while ((c = fgetc(infile)) == '#')
    while ((c = fgetc(infile)) != '\n')
      ;
  ungetc(c, infile);
  fscanf(infile, "%d %d\n255\n", &width, &height);
  size = width * height;

  // allocate space for all pixels in the image
  gval = malloc(size * sizeof(Pixel));
  if (gval == NULL)
  {
    fprintf(stderr, "Out of memory!");
    fclose(infile);
    return (0);
  }
  // read all the pixels
  fread(gval, sizeof(Pixel), size, infile);

  fclose(infile);
  return (1);
} /* ImagePGMBinRead */

/**
 * @brief Reads contents of a given ppm image
 *
 * @param fname Path to the ppm file
 * @return short 0 on failure, 1 otherwise
 */
short ImagePPMRead(char *fname)
{
  FILE *infile;
  char id[4];

  infile = fopen(fname, "r");
  if (infile == NULL)
  {
    fprintf(stderr, "Error: Can't read the image: %s !", fname);
    return (0);
  }
  // read ppm header
  fscanf(infile, "%3s", id);
  fclose(infile);
  // check ppm header and call corresponding function
  if (strcmp(id, "P3") == 0)
    return (ImagePPMAsciiRead(fname));
  else if (strcmp(id, "P6") == 0)
    return (ImagePPMBinRead(fname));
  else
  {
    fprintf(stderr, "Unknown type of the image!");
    return (0);
  }
} /* ImagePPMRead */

int ImagePPMBinWrite(char *fname)
{
  FILE *outfile;

  outfile = fopen(fname, "wb");
  if (outfile == NULL)
  {
    fprintf(stderr, "Error: Can't write the image: %s !", fname);
    return (-1);
  }
  fprintf(outfile, "P6\n%d %d\n255\n", width, height);

  fwrite(out, sizeof(Pixel), (size_t)(size), outfile);

  fclose(outfile);
  return (0);
} /* ImagePPMBinWrite */

int main(int argc, char *argv[])
{

  char *imgfname, *outfname = "out.ppm";
  int r;
  unsigned long i;
  clock_t start;
  struct tms tstruct;
  long tickspersec = sysconf(_SC_CLK_TCK);
  float musec;
  SalienceTree *tree;

  // Check if the right amount of arguments are provided and set variables accirding to them
  if (argc < 3)
  {
    printf("Usage: %s <input image> <lambda>  [omegafactor] [output image] \n", argv[0]);
    exit(0);
  }

  imgfname = argv[1];

  lambda = atoi(argv[2]);
  if (argc > 3)
    omegafactor = atof(argv[3]);

  if (argc > 4)
    outfname = argv[4];

  // Read the input image
  // This sets both the global gval pixel array (input image)
  // as well as the dimensions of the image (height, width, size)
  if (!ImagePPMRead(imgfname))
    return (-1);

  // allocate space for the pixel array thata is the output image
  out = malloc(size * sizeof(Pixel));

  printf("Filtering image '%s' using attribute area with lambda=%d\n", imgfname, lambda);
  printf("Image: Width=%d Height=%d\n", width, height);

  printf("Data read, start filtering.\n");
  start = times(&tstruct);
  // create the actual alpha tree
  tree = MakeSalienceTree(gval, width, height, (double)lambda);

  musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

  printf("wall-clock time: %f s\n", musec);
  // apply what we have found in the alpha tree creation to the out image
  // here colors and areas are created etc.
  // SalienceTreeAreaFilter(tree,out,lambda);
  SalienceTreeSalienceFilter(tree, out, (double)lambda);

  musec = (float)(times(&tstruct) - start) / ((float)tickspersec);

  printf("wall-clock time: %f s\n", musec);

  r = ImagePPMBinWrite(outfname);
  free(out);
  if (r)
    printf("Filtered image written to '%s'\n", outfname);

  free(gval);
  return (0);
} /* main */
