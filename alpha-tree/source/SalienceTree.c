#include "SalienceTree.h"
#include "../util/EdgeDetection.h"
#include "../util/DistanceMeasures.h"
#include <stdlib.h>
#include <assert.h>

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
 * @brief Create a Salience Node object in a given tree
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
  int parent = tree->node[i].parent;

  if (parent == BOTTOM)
    return true;
  return (tree->node[i].alpha != tree->node[parent].alpha);
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
    edgeSalience = edgeStrengthX(img, width, height, x, 0, salienceFunction);
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
    edgeSalience = edgeStrengthY(img, width, height, 0, y, salienceFunction);

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
      edgeSalience = edgeStrengthY(img, width, height, x, y, salienceFunction);
      if (edgeSalience < lambdamin)
      {
        Union(tree, root, p, p - width);
      }
      else
      {
        EdgeQueuePush(queue, p, p - width, edgeSalience);
      }
      // repeat process in x-direction
      edgeSalience = edgeStrengthX(img, width, height, x, y, salienceFunction);
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
