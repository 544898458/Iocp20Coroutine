#include <vld.h>
#include <glog/logging.h>
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include <thread>
#include"./CoRoutine/CoTask.h"

static int t = 1;
#define T 9  

using namespace std;

typedef int Status;
CoTask<int> PostSend()
{
	LOG(INFO) << "PostSend";
	bool needYield, callSend;
	co_yield 0;
	LOG(INFO) << ("PostSend end\n");
}
void NetworkThreadProc(CoTask<int>* co)
{
	LOG(INFO) << ("NetworkThreadProc\n");
	co->Run();
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	Status i = 1;
	//LOG(INFO) << T * i << endl; //Test Cout  
	CoTask<int> co = PostSend();
	std::thread networkThread(NetworkThreadProc, &co);
	networkThread.detach();
	std::this_thread::sleep_for(2000ms);
	std::thread networkThread2(NetworkThreadProc, &co);
	networkThread2.detach();
	std::this_thread::sleep_for(2000ms);

	_CrtDumpMemoryLeaks();	 //显示内存泄漏报告
	return 0;
}