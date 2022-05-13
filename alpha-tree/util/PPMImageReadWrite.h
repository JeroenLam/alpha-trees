#ifndef PPM_IMAGE_READ_WRITE_H
#define PPM_IMAGE_READ_WRITE_H

short ImagePPMAsciiRead(char *fname);
short ImagePPMBinRead(char *fname);
short ImagePPMRead(char *fname);
int ImagePPMBinWrite(char *fname);

#endif