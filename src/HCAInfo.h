#ifndef HCADECODER_SRC_INFO_H
#define HCADECODER_SRC_INFO_H

typedef struct HCAInfo {
  unsigned short versionMajor;
  unsigned short versionMinor;
  unsigned int channelCount;
  unsigned int samplingRate;
  unsigned int blockCount;
  unsigned int muteHeader;
  unsigned int muteFooter;

  int comp;
  int dec;
  int vbr;
  float bitRate;
  unsigned int blockSize;
  unsigned int comp1;
  unsigned int comp2;
  unsigned int comp3;
  unsigned int comp4;
  unsigned int comp5;
  unsigned int comp6;
  unsigned int comp7;
  unsigned int comp8;

  unsigned int dec1;
  unsigned int dec2;
  unsigned int dec3;
  unsigned int dec4;
  unsigned int dec5;
  unsigned int dec6;
  unsigned int dec7;

  unsigned int vbr1;
  unsigned int vbr2;

  unsigned int athType;

  int loop;
  unsigned int loopStart;
  unsigned int loopEnd;
  unsigned int loopCount; // 0x80 infinite
  unsigned int loop1;

  unsigned int ciphType;
  float rvaVolume;
} HCAInfo;

const char* getInfoErrorMessage (int code);

#endif
