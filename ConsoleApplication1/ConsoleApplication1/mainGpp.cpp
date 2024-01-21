#include <iostream>  
#include <thread>
#include"./CoRoutine/CoTask.h"

static int t = 1;
#define T 9  

using namespace std;

typedef int Status;
CoTask<int> PostSend()
{
	printf("PostSend\n");
	bool needYield, callSend;
	co_yield 0;
	printf("PostSend end\n");
}
void NetworkThreadProc(CoTask<int>* co)
{
	printf("NetworkThreadProc\n");
	co->Run();
}

int main()
{
	Status i = 1;
	cout << T * i << endl; //Test Cout  
	CoTask<int> co = PostSend();
	std::thread networkThread(NetworkThreadProc, &co);
	networkThread.detach();
	std::this_thread::sleep_for(2000ms);
	std::thread networkThread2(NetworkThreadProc, &co);
	networkThread2.detach();
	std::this_thread::sleep_for(2000ms);
	return 0;
}