#ifndef HCADECODESERVICE_H
#define HCADECODESERVICE_H

#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Semaphore.h"
#include "clHCA.h"

class HCADecodeService
{
public:
	HCADecodeService();
	HCADecodeService(unsigned int num_threads, unsigned int chunksize = 24);
	~HCADecodeService();
	void cancel_decode(void *ptr);
	std::pair<void *, size_t> decode(const char *hcafilename, unsigned int decodefromsample = 0, unsigned int ciphKey1 = 0xBC731A85, unsigned int ciphKey2 = 0x0002B875, unsigned int subKey = 0, float volume = 1.0f, int mode = 16, int loop = 0);
	void wait_on_request(void *ptr);
	void wait_for_finish();
	static bool print_info(const char *filenameHCA, unsigned int ciphKey1, unsigned int ciphKey2);
private:
	void Main_Thread();
	void Decode_Thread(unsigned int id);
	void load_next_request();
	void populate_block_list();
	void join_workers();

	clHCA workingfile;
	unsigned int numthreads, numchannels, chunksize, startingblock, blocklistsz, *blocks;
	void *workingrequest;
	std::thread dispatchthread;
	std::thread *workerthreads;
	std::map<void *, std::pair<clHCA, unsigned int>> filelist;
	Semaphore *workersem;
	Semaphore datasem, mainsem;
	std::mutex filelistmtx, workingmtx;
	std::condition_variable workingcv;
	clHCA::stChannel *channels;
	float *wavebuffer;
	bool shutdown, stopcurrent;
	std::atomic<unsigned int> currindex;
};

#endif //HCADECODESERVICE_H
