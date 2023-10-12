#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include <Winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include "Accept.h"


BOOL g_flag = TRUE;
Iocp::Accept *g_Accept = nullptr;
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT://控制台关闭（点右上角X关闭）
		delete g_Accept;
		g_Accept = nullptr;
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
DWORD WINAPI ThreadProc(LPVOID lpParameter);

int main(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	SetConsoleCtrlHandler(fun, TRUE);
	//WORD wdVersion = MAKEWORD(2, 2);
	//WSADATA wdScokMsg;
	//int nRes = WSAStartup(wdVersion, &wdScokMsg);

	//if (0 != nRes)
	//{
	//	switch (nRes)
	//	{
	//	case WSASYSNOTREADY:
	//		printf("重启下电脑试试，或者检查网络库");
	//		break;
	//	case WSAVERNOTSUPPORTED:
	//		printf("请更新网络库");
	//		break;
	//	case WSAEINPROGRESS:
	//		printf("请重新启动");
	//		break;
	//	case WSAEPROCLIM:
	//		printf("请尝试关掉不必要的软件，以为当前网络运行提供充足资源");
	//		break;
	//	}
	//	return  0;
	//}

	////校验版本
	//if (2 != HIBYTE(wdScokMsg.wVersion) || 2 != LOBYTE(wdScokMsg.wVersion))
	//{
	//	//说明版本不对
	//	//清理网络库
	//	WSACleanup();
	//	return 0;
	//}
	auto accept = new Iocp::Accept();
	accept->WsaStartup();
	accept->Init();



	//阻塞
	while (true)
	{
		Sleep(1000);
	}

	//CloseHandle(hPort);
	//Clear();
	////清理网络库
	//WSACleanup();

	//system("pause");
	return 0;
}
