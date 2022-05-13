#include "PPMImageReadWrite.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/**
 * @brief Writes the contents of the Pixel array representing the output image
 * to a PPM image file.
 * 
 * @param fname Name of the output PPM image
 * @return int 0 on success, -1 on failure
 */
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