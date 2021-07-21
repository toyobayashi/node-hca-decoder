#ifndef __CL_HCA_H__
#define __CL_HCA_H__

#include <stddef.h>

// Copyright (c) 2017 にゃが

//--------------------------------------------------
// HCA(High Compression Audio)クラス
//--------------------------------------------------
class clHCA {
public:
    clHCA(unsigned int ciphKey1 = 0xBC731A85, unsigned int ciphKey2 = 0x0002B875, unsigned int subKey = 0);
    clHCA &operator=(clHCA &&other) noexcept;
    ~clHCA();

    // HCAチェック
    static bool CheckFile(void *data, unsigned int size);

    // チェックサム
    static unsigned short CheckSum(void *data, int size, unsigned short sum = 0);

    // ヘッダ情報をコンソール出力
    bool PrintInfo(const char *filenameHCA);

    // 復号化
    bool Decrypt(const char *filenameHCA);

    unsigned int get_channelCount() const;
    unsigned int get_blockCount() const;
    unsigned int get_blockSize() const;

private:
    struct stHeader {//ファイル情報 (必須)
        unsigned int hca;              // 'HCA'
        unsigned short version;        // バージョン。v1.3とv2.0の存在を確認
        unsigned short dataOffset;     // データオフセット
    };
    struct stFormat {//フォーマット情報 (必須)
        unsigned int fmt;              // 'fmt'
        unsigned int channelCount : 8;   // チャンネル数 1～16
        unsigned int samplingRate : 24;  // サンプリングレート 1～0x7FFFFF
        unsigned int blockCount;       // ブロック数 0以上
        unsigned short muteHeader;     // 先頭の無音部分(ブロック数*0x400+0x80)
        unsigned short muteFooter;     // 末尾の無音サンプル数
    };
    struct stCompress {//圧縮情報 (圧縮情報かデコード情報のどちらか一つが必須)
        unsigned int comp;             // 'comp'
        unsigned short blockSize;      // ブロックサイズ(CBRのときに有効？) 8～0xFFFF、0のときはVBR
        unsigned char r01;             // 不明(1) 0～r02      v2.0現在1のみ対応
        unsigned char r02;             // 不明(15) r01～0x1F  v2.0現在15のみ対応
        unsigned char r03;             // 不明(1)(1)
        unsigned char r04;             // 不明(1)(0)
        unsigned char r05;             // 不明(0x80)(0x80)
        unsigned char r06;             // 不明(0x80)(0x20)
        unsigned char r07;             // 不明(0)(0x20)
        unsigned char r08;             // 不明(0)(8)
        unsigned char reserve1;        // 予約
        unsigned char reserve2;        // 予約
    };
    struct stDecode {//デコード情報 (圧縮情報かデコード情報のどちらか一つが必須)
        unsigned int dec;              // 'dec'
        unsigned short blockSize;      // ブロックサイズ(CBRのときに有効？) 8～0xFFFF、0のときはVBR
        unsigned char r01;             // 不明(1) 0～r02      v2.0現在1のみ対応
        unsigned char r02;             // 不明(15) r01～0x1F  v2.0現在15のみ対応
        unsigned char count1;          // type0とtype1の数-1
        unsigned char count2;          // type2の数-1
        unsigned char r03 : 4;           // 不明(0)
        unsigned char r04 : 4;           // 不明(0) 0は1に修正される
        unsigned char enableCount2;    // count2を使うフラグ
    };
    struct stVBR {//可変ビットレート情報 (廃止？)
        unsigned int vbr;              // 'vbr'
        unsigned short r01;            // 不明 0～0x1FF
        unsigned short r02;            // 不明
    };
    struct stATH {//ATHテーブル情報 (v2.0から廃止？)
        unsigned int ath;              // 'ath'
        unsigned short type;           // テーブルの種類(0:全て0 1:テーブル1)
    };
    struct stLoop {//ループ情報
        unsigned int loop;             // 'loop'
        unsigned int start;            // ループ開始ブロックインデックス 0～loopEnd
        unsigned int end;              // ループ終了ブロックインデックス loopStart～(stFormat::blockCount-1)
        unsigned short count;          // ループ回数 0x80で無限ループ
        unsigned short r01;            // 不明(0x226) 
    };
    struct stCipher {//暗号テーブル情報
        unsigned int ciph;             // 'ciph'
        unsigned short type;           // 暗号化の種類(0:暗号化なし 1:鍵なし暗号化 0x38:鍵あり暗号化)
    };
    struct stRVA {//相対ボリューム調節情報
        unsigned int rva;              // 'rva'
        float volume;                  // ボリューム
    };
    struct stComment {//コメント情報
        unsigned int comm;             // 'comm'
        unsigned char len;             // コメントの長さ？
                                       //char comment[];
    };
    struct stPadding {//パディング
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
    float _volume;
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
        ~clData();
        int CheckBit(int bitSize);
        int GetBit(int bitSize);
        void AddBit(int bitSize);
        unsigned char *getData();
    private:
        unsigned char *_data;
        int _size;
        int _bit;
    };
    public:
    struct stChannel {
        float block[0x80];
        float base[0x80];
        unsigned char value[0x80];
        unsigned char scale[0x80];
        unsigned char value2[8];
        int type;
        unsigned char *value3;
        unsigned int count;
        float wav1[0x80];
        float wav2[0x80];
        void Decode1(clData *data, unsigned int a, int b, unsigned char *ath);
        void Decode2(clData *data);
        void Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
        void Decode4(int index, unsigned int a, unsigned int b, unsigned int c);
        void Decode5(float* wavebuffer, unsigned int channelCount, float volume);
    };
    bool PrepDecode(stChannel* channels);
    bool Analyze(void *&wavptr, size_t &sz, const char *filenameHCA, float volume = 1.0f, int mode = 16, int loop = 0);
    void AsyncDecode(stChannel *channels, float *wavebuffer, unsigned int blocknum, void *outputwavptr, unsigned int chunksize, bool &stop);
    private:
    unsigned int _mode;
    unsigned int _loopNum;
    unsigned char *hcafileptr;
    unsigned int _wavheadersize;
    bool Decode(void *data, unsigned int size, unsigned int address);
    void (*_modeFunction)(clHCA *, unsigned int, void *, float *);
    static void DecodeToMemory_DecodeModeFloat(float f, void *ptr);
    static void DecodeToMemory_DecodeMode8bit (float f, void *ptr);
    static void DecodeToMemory_DecodeMode16bit(float f, void *ptr);
    static void DecodeToMemory_DecodeMode24bit(float f, void *ptr);
    static void DecodeToMemory_DecodeMode32bit(float f, void *ptr);
    static void DecodeToMemory_DecodeModeFloat(clHCA *clhca, unsigned int currblock, void *ptr, float *buffer);
    static void DecodeToMemory_DecodeMode8bit (clHCA *clhca, unsigned int currblock, void *ptr, float *buffer);
    static void DecodeToMemory_DecodeMode16bit(clHCA *clhca, unsigned int currblock, void *ptr, float *buffer);
    static void DecodeToMemory_DecodeMode24bit(clHCA *clhca, unsigned int currblock, void *ptr, float *buffer);
    static void DecodeToMemory_DecodeMode32bit(clHCA *clhca, unsigned int currblock, void *ptr, float *buffer);
};

#endif
