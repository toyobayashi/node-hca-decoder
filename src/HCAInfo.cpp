#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#include <stdio.h>
#include <string.h>

#include "clHCA.h"

int clHCA::GetInfo(const char* filenameHCA, HCAInfo* info) {
  if (!(info)) return 1;
  memset(info, 0, sizeof(HCAInfo));
  if (!(filenameHCA)) return 2;

  FILE *fp;
#ifdef _WIN32
  wchar_t strUnicode[260];
  MultiByteToWideChar(CP_UTF8, 0, filenameHCA, -1, strUnicode, 260);
  if (!(fp = _wfopen(strUnicode, L"rb"))) {
    return 2;
  }
#else
  if (!(fp = fopen(filenameHCA, "rb"))) {
    return 2;
  }
#endif

  stHeader header;
  memset(&header, 0, sizeof(header));
  fread(&header, sizeof(header), 1, fp);
  if (!CheckFile(&header, sizeof(header))) {
    fclose(fp); return 3;
  }

  header.dataOffset = bswap(header.dataOffset);
  unsigned char *data = new unsigned char[header.dataOffset];
  if (!data) {
    fclose(fp); return 4;
  }
  fseek(fp, 0, SEEK_SET);
  fread(data, header.dataOffset, 1, fp);

  unsigned char *s = (unsigned char *)data;
  unsigned int size = header.dataOffset;

  if (size<sizeof(stHeader)) {
    // printf("Error: Header size too small.\n");
    delete[] data; fclose(fp); return 5;
  }

  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00414348) {
    stHeader *hca = (stHeader *)s; s += sizeof(stHeader);
    unsigned int version = bswap(hca->version);
    // unsigned int dataOffset = bswap(hca->dataOffset);
    info->versionMajor = version >> 8;
    info->versionMinor = version & 0xFF;
    // if (CheckSum(hca, dataOffset))printf("[!] Bad header.\n");
  }

  // fmt
  unsigned int samplingRate = 0;
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00746D66) {
    stFormat *fmt = (stFormat *)s; s += sizeof(stFormat);
    unsigned int channelCount = fmt->channelCount;
    samplingRate = bswap(fmt->samplingRate << 8);
    unsigned int blockCount = bswap(fmt->blockCount);
    unsigned int muteHeader = bswap(fmt->muteHeader);
    unsigned int muteFooter = bswap(fmt->muteFooter);
    info->channelCount = channelCount;
    info->samplingRate = samplingRate;
    info->blockCount = blockCount;
    info->muteHeader = (muteHeader - 0x80) / 0x400;
    info->muteFooter = muteFooter;
  }

  // comp
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706D6F63) {
    info->comp = 1;
    stCompress *comp = (stCompress *)s; s += sizeof(stCompress);
    unsigned int blockSize = bswap(comp->blockSize);
    unsigned int comp_r01 = comp->r01;
    unsigned int comp_r02 = comp->r02;
    unsigned int comp_r03 = comp->r03;
    unsigned int comp_r04 = comp->r04;
    unsigned int comp_r05 = comp->r05;
    unsigned int comp_r06 = comp->r06;
    unsigned int comp_r07 = comp->r07;
    unsigned int comp_r08 = comp->r08;
    unsigned int bps = samplingRate * blockSize / 128;
    info->bitRate = bps / 1000.0f; // kbps
    info->blockSize = blockSize;

    info->comp1 = comp_r01;
    info->comp2 = comp_r02;
    info->comp3 = comp_r03;
    info->comp4 = comp_r04;
    info->comp5 = comp_r05;
    info->comp6 = comp_r06;
    info->comp7 = comp_r07;
    info->comp8 = comp_r08;
  }

  // dec
  else if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00636564) {
    info->dec = 1;
    stDecode *dec = (stDecode *)s; s += sizeof(stDecode);
    unsigned int blockSize = bswap(dec->blockSize);
    unsigned int comp_r01 = dec->r01;
    unsigned int comp_r02 = dec->r02;
    unsigned int comp_r03 = dec->r04;
    unsigned int comp_r04 = dec->r03;
    unsigned int comp_r05 = dec->count1 + 1;
    unsigned int comp_r06 = ((dec->enableCount2) ? dec->count2 : dec->count1) + 1;
    unsigned int comp_r07 = comp_r05 - comp_r06;
    // unsigned int comp_r08 = 0;
    unsigned int bps = samplingRate * blockSize / 128;
    info->bitRate = bps / 1000.0f; // kbps
    info->blockSize = blockSize;

    info->dec1 = comp_r01;
    info->dec2 = comp_r02;
    info->dec3 = comp_r03;
    info->dec4 = comp_r04;
    info->dec5 = comp_r05;
    info->dec6 = comp_r06;
    info->dec7 = comp_r07;
  }
  // else {
  //   printf("[!] comp or dec missing.\n");
  // }

  // vbr
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00726276) {
    info->vbr = 1;
    stVBR *vbr = (stVBR *)s; s += sizeof(stVBR);
    unsigned int vbr_r01 = bswap(vbr->r01);
    unsigned int vbr_r02 = bswap(vbr->r02);
    info->vbr1 = vbr_r01;
    info->vbr2 = vbr_r02;
  }
  else {
    info->vbr1 = 0;
    info->vbr2 = 0;
  }

  // ath
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00687461) {
    stATH *ath = (stATH *)s; s += 6;//s+=sizeof(stATH);
    info->athType = ath->type;
  }

  // loop
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x706F6F6C) {
    info->loop = 1;
    stLoop *loop = (stLoop *)s; s += sizeof(stLoop);
    unsigned int loopStart = bswap(loop->start);
    unsigned int loopEnd = bswap(loop->end);
    unsigned int loopCount = bswap(loop->count);
    unsigned int loop_r01 = bswap(loop->r01);
    info->loopStart = loopStart;
    info->loopEnd = loopEnd;
    info->loopCount = loopCount;
    info->loop1 = loop_r01;
  }

  // ciph
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x68706963) {
    stCipher *ciph = (stCipher *)s; s += 6;//s+=sizeof(stCipher);
    unsigned int ciph_type = bswap(ciph->type);
    info->ciphType = ciph_type;
  }

  // rva
  if ((*(unsigned int *)s & 0x7F7F7F7F) == 0x00617672) {
    stRVA *rva = (stRVA *)s; s += sizeof(stRVA);
    info->rvaVolume = bswap(rva->volume);
  }

  delete[] data;

  fclose(fp);

  return 0;
}

const char* getInfoErrorMessage (int code) {
  switch (code) {
  case 0: return "OK";
  case 1: return "Null info";
  case 2: return "HCA open failed";
  case 3: return "Invalid HCA";
  case 4: return "Out of memory";
  case 5: return "Invalid header";
  default: return "Unknown error";
  }
}
