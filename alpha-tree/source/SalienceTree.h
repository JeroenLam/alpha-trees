#ifndef SALIENCE_TREE_H
#define SALIENCE_TREE_H

#include "../util/common.h"
#include "EdgeQueue.h"

#define Par(tree, p) LevelRoot(tree, tree->node[p].parent)

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
  SalienceNode *node;
} SalienceTree;


SalienceTree *CreateSalienceTree(int imgsize);
SalienceTree *MakeSalienceTree(Pixel *img, int width, int height, double lambdamin);
void DeleteTree(SalienceTree *tree);
int NewSalienceNode(SalienceTree *tree, int *root, double alpha);
int FindRoot(int *root, int p);
int FindRoot1(SalienceTree *tree, int *root, int p);
int LevelRoot(SalienceTree *tree, int p);
boolean IsLevelRoot(SalienceTree *tree, int i);
void MakeSet(SalienceTree *tree, int *root, Pixel *gval, int p);
void GetAncestors(SalienceTree *tree, int *root, int *p, int *q);
void Union(SalienceTree *tree, int *root, int p, int q);
void Union2(SalienceTree *tree, int *root, int p, int q);
void Phase1(SalienceTree *tree, EdgeQueue *queue, int *root, Pixel *img, int width, int height, double lambdamin);
void Phase2(SalienceTree *tree, EdgeQueue *queue, int *root, Pixel *img, int width, int height);

#endif