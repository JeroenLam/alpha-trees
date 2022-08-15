#include "PPMImageReadWrite.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

/**
 * @brief Reads contents of a given ppm image into the global gval Pixel array.
 * The ppm image should be encoded in ASCII format. It contains the P3 ppm header signature.
 *
 * @param fname Path to the ppm image to read
 * @return short 0 on failure, 1 otherwise
 */
short ImagePPMAsciiRead(string fname, int *width, int *height)
{
  FILE *infile;
  unsigned long i, j;
  int c;

  infile = fopen(fname.c_str(), "r");
  if (infile == NULL)
  {
    cerr << "Error: Can't read the ASCII file: " << fname;
    return (0);
  }
  fscanf(infile, "P3\n");
  // skip comments
  while ((c = fgetc(infile)) == '#')
    while ((c = fgetc(infile)) != '\n')
      ;
  ungetc(c, infile);

  // scan width and height from ppm and assume color spectrum [0,255]
  fscanf(infile, "%d %d\n255\n", width, height);
  int size = *width * *height;

  // allocate space for all pixels in the image
  gval = (Pixel*) malloc(size * sizeof(Pixel));
  if (gval == NULL)
  {
    cerr << "Out of memory!";
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
short ImagePPMBinRead(string fname, int *width, int *height)
{
	FILE * infile;
	int c, i;


	infile = fopen(fname.c_str(), "rb");
	if (infile == NULL)
	{
		cerr << "Error: Can't read the binary file:"<< fname << "\n";
		return (0);
	}
	fscanf(infile, "P6\n");
	// skip comments
	while ((c = fgetc(infile)) == '#')
	while ((c = fgetc(infile)) != '\n')
	;
	ungetc(c, infile);
	fscanf(infile, "%d %d\n255\n", width, height);
	int size = *width * *height;

	// allocate space for all pixels in the image
	gval = (Pixel*) malloc(size * sizeof(Pixel));
	if (gval == NULL)
	{
		cerr << "Out of memory!";
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
short ImagePPMRead(string fname, int *width, int *height)
{
	FILE *infile;
	char id[4];

	infile = fopen(fname.c_str(), "r");
	if (infile == NULL)
	{
		cerr << "Error: Can't read the image: " << fname;
		return (0);
	}
	// read ppm header
	fscanf(infile, "%3s", id);
	fclose(infile);
	// check ppm header and call corresponding function
	if (strcmp(id, "P3") == 0)
		return (ImagePPMAsciiRead(fname, width, height));
	else if (strcmp(id, "P6") == 0)
		return (ImagePPMBinRead(fname, width, height));
	else
	{
		cerr << "Unknown type of the image!";
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
int ImagePPMBinWrite(string fname, int width, int height)
{
	FILE *outfile;
	int size = width*height;

	outfile = fopen(fname.c_str(), "wb");
	if (outfile == NULL)
	{
		cerr << "Error: Can't write the image: " << fname;
		return (-1);
	}
	fprintf(outfile, "P6\n%d %d\n255\n", width, height);

	fwrite(out, sizeof(Pixel), (size_t)(size), outfile);

	fclose(outfile);
	return (0);
} /* ImagePPMBinWrite */
