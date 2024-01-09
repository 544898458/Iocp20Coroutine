#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include <Winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include "Server.h"
#include "MySession.h"
#include "MsgQueue.h"

BOOL g_flag = TRUE;
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT://控制台关闭（点右上角X关闭）
		//delete g_Accept;
		//g_Accept = nullptr;
		_CrtDumpMemoryLeaks();	 //显示内存泄漏报告
		//释放所有socket
		//CloseHandle(hPort);
		//Clear();

		g_flag = FALSE;

		//释放线程句柄
		//for (int i = 0; i < process_count; i++)
		//{
		//	//TerminateThread(pThread[i],);
		//	CloseHandle(pThread[i]);
		//}
		//free(pThread);

		break;
	}
	return TRUE;
}
//DWORD WINAPI ThreadProc(LPVOID lpParameter);
int main(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	SetConsoleCtrlHandler(fun, TRUE);
	
	auto accept = new Iocp::Server<MySession>();
	accept->WsaStartup();
	accept->Init();



	//主逻辑工作线程
	while (true)
	{
		Sleep(100);
		g_MsgQueue.Process();
	}


	return 0;
}
