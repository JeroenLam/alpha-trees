#include "TreeFilter.h"
#include "util.h"

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
    // if lambda is larger than the root alpha we get a black image because the tree is not that deep
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
    // if lambda is larger than the root alpha we get a black image because the tree is not that deep
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

// NOT READY
void SalienceTreeColorMapFilter(SalienceTree *tree, Pixel *out, double lambda)
{
  int i, j, imgsize = tree->maxSize / 2;
  printf("\nTree Summary:\n");
  printf("Max:%d; Curr:%d\n", tree->maxSize, tree->curSize);
  SalienceNode curr;
  // for (int i = 0; i < tree->curSize; i++)
  // {
  //   curr = tree->node[i];
  //   printf("__________________\n");
  //   printf("Node no.%d\n", i);
  //   printf("Parent:%d\n", curr.parent);
  //   printf("Alpha:%f\n", curr.alpha);
  //   printf("Max:");
  //   printPixel(curr.maxPix);
  //   printf(" Min:");
  //   printPixel(curr.maxPix);
  //   printf("\n");
  //   printf("Area:%d\n", curr.area);
  //   printf("LevelRoot:%d\n", IsLevelRoot(tree, i));
  //   printf("Depth:%d\n", Depth(tree, i));
  //   printf("__________________\n");
  // }
  int max_depth = 2;
  for (j = 0; j < 3; j++)
  {
    tree->node[tree->curSize - 1].outval[j] =
        255;
  }
  for (i = tree->curSize - 2; i >= 0; i--)
  {
    //if (IsLevelRoot(tree, i) && Depth(tree, i) <= max_depth)
    {
      for (j = 0; j < 3; j++)
      {
        tree->node[i].outval[j] = 255 / (double)Depth(tree, i);
      }
    }
    // else
    // {
    //   for (j = 0; j < 3; j++)
    //   {
    //     tree->node[i].outval[j] = tree->node[tree->node[i].parent].outval[j];
    //   }
    // }
  }
  // set colors of the out image
  for (i = 0; i < imgsize; i++)
    for (j = 0; j < 3; j++)
      out[i][j] = tree->node[i].outval[j];
}
