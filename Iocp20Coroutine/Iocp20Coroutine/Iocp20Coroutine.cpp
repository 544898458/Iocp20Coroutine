#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#include "IocpNetwork/Server.h"
#include "MySession.h"
#include "MsgQueue.h"
#include "Space.h"
#include "Entity.h"

#include <glog/logging.h>
#pragma comment(lib, "glog.lib")

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
CoTask<int> Patrol(Entity* pEntity,float &x)
{
	while (true)
	{
		co_yield 0;

		x += 0.01f;

		MsgNotifyPos msg  {(int)NotifyPos ,(uint64_t)pEntity, x};
		Broadcast(msg);
	}
}


Space space;

///*
int main(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);

	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216
	google::InitGoogleLogging("test");//使用glog之前必须先初始化库，仅需执行一次，括号内为程序名

	LOG(INFO) << "Test GLOG_INFO";
	LOG(WARNING) << "Test GLOG_WARNING";
	LOG(ERROR) << "Test GLOG_ERROR";

	SetConsoleCtrlHandler(fun, TRUE);
	
	auto accept = new Iocp::Server<MySession>();
	accept->WsaStartup();
	accept->Init();


	//space.mapEntity[0] = new Entity(-5,space, Patrol);
	//space.mapEntity[1] = new Entity(5, space, TraceEnemy);
	//主逻辑工作线程
	while (true)
	{
		Sleep(100);
		std::set<Iocp::SessionSocketCompeletionKey<MySession>*> setDelete;
		for (auto p : g_set) 
		{
			p->Session.msgQueue.Process();
			if (p->Finished())
				setDelete.insert(p);
		}
		for (auto p : setDelete) 
		{
			p->Session.OnDestroy();
			delete p;
			LOG(INFO) << "已删除对象,GetCurrentThreadId=" << GetCurrentThreadId();

		}
		space.Update();
	}


	return 0;
}
//*/