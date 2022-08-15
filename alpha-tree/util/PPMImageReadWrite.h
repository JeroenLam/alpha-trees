#ifndef PPM_IMAGE_READ_WRITE_H
#define PPM_IMAGE_READ_WRITE_H
#include <string>
using namespace std;

short ImagePPMAsciiRead(string fname, int *width, int *height);
short ImagePPMBinRead(string fname, int *width, int *height);
short ImagePPMRead(string fname, int *width, int *height);
int ImagePPMBinWrite(string fname, int width, int height);

#endif
