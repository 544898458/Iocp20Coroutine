#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include <Winsock2.h>
#include <mswsock.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include "IocpNetwork/Server.h"
#include "MySession.h"
#include "MsgQueue.h"
#include "Space.h"
#include "Entity.h"

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
CoTask<int> Patrol(float &x)
{
	while (true)
	{
		co_yield 0;

		x += 0.1;

		MsgNotifyPos msg = { x };
		Broadcast(msg);
	}
}

CoTask<int> TraceEnemy(float& x)
{
	while (true)
	{
		co_yield 0;

		x += 0.1;

		MsgNotifyPos msg = { x };
		Broadcast(msg);
	}
}


int main(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	SetConsoleCtrlHandler(fun, TRUE);
	
	auto accept = new Iocp::Server<MySession>();
	accept->WsaStartup();
	accept->Init();


	Space space;
	space.mapEntity[0] = new Entity(-5,space, Patrol);
	space.mapEntity[1] = new Entity(5, space, TraceEnemy);
	//主逻辑工作线程
	while (true)
	{
		Sleep(1000);
		for (auto p : g_set) 
		{
			p->msgQueue.Process();
		}
		space.Update();
	}


	return 0;
}
