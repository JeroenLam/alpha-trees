#ifndef TREE_FILTER_H
#define TREE_FILTER_H

#include "common.h"
#include "../source/SalienceTree.h"

#define Par(tree, p) LevelRoot(tree, tree->node[p].parent)
#define NodeSalience(tree, p) (tree->node[Par(tree, p)].alpha)

void SalienceTreeAreaFilter(SalienceTree *tree, Pixel *out, int lambda);
void SalienceTreeSalienceFilter(SalienceTree *tree, Pixel *out, double lambda);

#endif