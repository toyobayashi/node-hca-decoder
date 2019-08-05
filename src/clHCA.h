#ifndef __CL_HCA_H__
#define __CL_HCA_H__

//--------------------------------------------------
// HCA(High Compression Audio) Class
//--------------------------------------------------
class clHCA {
public:
  clHCA(unsigned int ciphKey1 = 0xE0748978, unsigned int ciphKey2 = 0xCF222F1F);

  // HCA Check
  static bool CheckFile(void *data, unsigned int size);

  // CheckSum
  static unsigned short CheckSum(void *data, int size, unsigned short sum = 0);

  // Print header information
  bool PrintInfo(const char *filenameHCA);

  // Decrypt
  bool Decrypt(const char *filenameHCA);

  // Decode to WAV file
  bool DecodeToWavefile(const char *filenameHCA, const char *filenameWAV, float volume = 1, int mode = 16, int loop = 0);
  bool DecodeToWavefileStream(void *fpHCA, const char *filenameWAV, float volume = 1, int mode = 16, int loop = 0);

  // Encode to HCA file
  //bool EncodeFromWavefile(const char *filenameWAV,const char *filenameHCA,float volume=1);
  //bool EncodeFromWavefileStream(void *fpWAV,const char *filenameHCA,float volume=1);

private:
  struct stHeader { // file information (required)
    unsigned int hca;              // 'HCA'
    unsigned short version;        // Version (1.3 or 2.0)
    unsigned short dataOffset;     // data offset
  };
  struct stFormat { // format information (required)
    unsigned int fmt;              // 'fmt'
    unsigned int channelCount : 8;   // chanel count 1~16
    unsigned int samplingRate : 24;  // sampling rate 1~0x7FFFFF
    unsigned int blockCount;       // block count >0
    unsigned short muteHeader;     // header no sound part (blockCount * 0x400 + 0x80)
    unsigned short muteFooter;     // footer no sound sample
  };
  struct stCompress { // compress information (require this or decode information)
    unsigned int comp;             // 'comp'
    unsigned short blockSize;      // block size (when CBR available?) 8~0xFFFF, 0 is VBR
    unsigned char r01;             // unknown(1) 0~r02      v2.0 now 1 only
    unsigned char r02;             // unknown(15) r01~0x1F  v2.0 now 15 only
    unsigned char r03;             // unknown(1)(1)
    unsigned char r04;             // unknown(1)(0)
    unsigned char r05;             // unknown(0x80)(0x80)
    unsigned char r06;             // unknown(0x80)(0x20)
    unsigned char r07;             // unknown(0)(0x20)
    unsigned char r08;             // unknown(0)(8)
    unsigned char reserve1;
    unsigned char reserve2;
  };
  struct stDecode { // decode information (require this or compress information)
    unsigned int dec;              // 'dec'
    unsigned short blockSize;      // block size (when CBR available?) 8~0xFFFF, 0 is VBR
    unsigned char r01;             // unknown(1) 0~r02      v2.0 now 1 only
    unsigned char r02;             // unknown(15) r01~0x1F  v2.0 now 15 only
    unsigned char count1;          // type0 and type1 count - 1
    unsigned char count2;          // type2 count - 1
    unsigned char r03 : 4;         // unknown(0)
    unsigned char r04 : 4;         // unknown(0) 0 is corrected to 1
    unsigned char enableCount2;    // flag of using count2
  };
  struct stVBR { // variable bit rate information (unused?)
    unsigned int vbr;              // 'vbr'
    unsigned short r01;            // unknown 0~0x1FF
    unsigned short r02;            // unknown
  };
  struct stATH { // ATH table information (unused from v2.0?)
    unsigned int ath;              // 'ath'
    unsigned short type;           // table type (0:all is 0, 1:table 1)
  };
  struct stLoop { //loop information
    unsigned int loop;             // 'loop'
    unsigned int start;            // loop start block index 0~loopEnd
    unsigned int end;              // loop end block index loopStart~(stFormat::blockCount-1)
    unsigned short count;          // loop count, 0x80 is infinite loop
    unsigned short r01;            // unknown(0x226) 
  };
  struct stCipher { // cipher table information
    unsigned int ciph;             // 'ciph'
    unsigned short type;           // cipher type (0:no 1:no key 0x38:key)
  };
  struct stRVA { // relative volume information
    unsigned int rva;              // 'rva'
    float volume;                  // volume
  };
  struct stComment { // comment information
    unsigned int comm;             // 'comm'
    unsigned char len;             // comment length?
                                   //char comment[];
  };
  struct stPadding { // padding
    unsigned int pad;              // 'pad'
  };
  unsigned int _version;
  unsigned int _dataOffset;
  unsigned int _channelCount;
  unsigned int _samplingRate;
  unsigned int _blockCount;
  unsigned int _muteHeader;
  unsigned int _muteFooter;
  unsigned int _blockSize;
  unsigned int _comp_r01;
  unsigned int _comp_r02;
  unsigned int _comp_r03;
  unsigned int _comp_r04;
  unsigned int _comp_r05;
  unsigned int _comp_r06;
  unsigned int _comp_r07;
  unsigned int _comp_r08;
  unsigned int _comp_r09;
  unsigned int _vbr_r01;
  unsigned int _vbr_r02;
  unsigned int _ath_type;
  unsigned int _loopStart;
  unsigned int _loopEnd;
  unsigned int _loopCount;
  unsigned int _loop_r01;
  bool _loopFlg;
  unsigned int _ciph_type;
  unsigned int _ciph_key1;
  unsigned int _ciph_key2;
  float _rva_volume;
  unsigned int _comm_len;
  char *_comm_comment;
  class clATH {
  public:
    clATH();
    bool Init(int type, unsigned int key);
    unsigned char *GetTable(void);
  private:
    unsigned char _table[0x80];
    void Init0(void);
    void Init1(unsigned int key);
  }_ath;
  class clCipher {
  public:
    clCipher();
    bool Init(int type, unsigned int key1, unsigned int key2);
    void Mask(void *data, int size);
  private:
    unsigned char _table[0x100];
    void Init0(void);
    void Init1(void);
    void Init56(unsigned int key1, unsigned int key2);
    void Init56_CreateTable(unsigned char *table, unsigned char key);
  }_cipher;
  class clData {
  public:
    clData(void *data, int size);
    int CheckBit(int bitSize);
    int GetBit(int bitSize);
    void AddBit(int bitSize);
  private:
    unsigned char *_data;
    int _size;
    int _bit;
  };
  struct stChannel {
    float block[0x80];
    float base[0x80];
    char value[0x80];
    char scale[0x80];
    char value2[8];
    int type;
    char *value3;
    unsigned int count;
    float wav1[0x80];
    float wav2[0x80];
    float wav3[0x80];
    float wave[8][0x80];
    void Decode1(clData *data, unsigned int a, int b, unsigned char *ath);
    void Decode2(clData *data);
    void Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
    void Decode4(int index, unsigned int a, unsigned int b, unsigned int c);
    void Decode5(int index);
  }_channel[0x10];
  bool Decode(void *data, unsigned int size, unsigned int address);
  bool DecodeToWavefile_Decode(void *fp1, void *fp2, unsigned int address, unsigned int count, void *data, void (*modeFunction)(float, void*));
  static void DecodeToWavefile_DecodeModeFloat(float f, void *fp);
  static void DecodeToWavefile_DecodeMode8bit(float f, void *fp);
  static void DecodeToWavefile_DecodeMode16bit(float f, void *fp);
  static void DecodeToWavefile_DecodeMode24bit(float f, void *fp);
  static void DecodeToWavefile_DecodeMode32bit(float f, void *fp);
};

#endif
