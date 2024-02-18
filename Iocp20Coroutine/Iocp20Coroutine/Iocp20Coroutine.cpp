#define _CRT_SECURE_NO_WARNINGS
#include <vld.h>
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
		LOG(WARNING) << "不能点右上角X关闭，可能有数据没保存";
		//delete g_Accept;
		//g_Accept = nullptr;
		
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
		Sleep(3000);
		_CrtDumpMemoryLeaks();	 //显示内存泄漏报告
		//Sleep(1000);
		//false;
		break;
	case CTRL_C_EVENT:
		g_flag = FALSE;
		//return false;
		break;
	}
	return TRUE;
}
CoTask<int> Patrol(Entity* pEntity,float &x, float& z)
{
	while (true)
	{
		co_yield 0;

		x += 0.01f;

		MsgNotifyPos msg  {(int)NotifyPos ,(uint64_t)pEntity, x,z};
		Broadcast(msg);
	}
}




///*
int main(void)
{
	//屏蔽控制台最小按钮和关闭按钮
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, false);
	RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);

	auto* pMemoryLeak = malloc(123);//测试内存泄漏
	const char sz[] = "Memory Leak Test.Nei Cun Xie Lou Jian Ce.This is not a real Defect.Zhe Bu Shi Yi Ge Zhen De Que Xian.";
	memcpy(pMemoryLeak, sz,sizeof(sz));
	pMemoryLeak = nullptr;

	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216
	google::InitGoogleLogging("test");//使用glog之前必须先初始化库，仅需执行一次，括号内为程序名

	LOG(INFO) << "Test GLOG_INFO";
	LOG(WARNING) << "Test GLOG_WARNING";
	LOG(ERROR) << "Test GLOG_ERROR";

	SetConsoleCtrlHandler(fun, TRUE);
	
	Iocp::Server<MySession> accept;
	accept.WsaStartup();
	accept.Init();


	//m_space.mapEntity[0] = new Entity(-5,m_space, Patrol);
	//m_space.mapEntity[1] = new Entity(5, m_space, TraceEnemy);
	//主逻辑工作线程
	while (g_flag)
	{
		Sleep(100);
		std::set<Iocp::SessionSocketCompeletionKey<MySession>*> setDelete;
		for (auto p : g_setSession) 
		{
			p->Session.m_msgQueue.Process();
			if (p->Finished())
				setDelete.insert(p);
		}
		for (auto p : setDelete) 
		{
			p->Session.OnDestroy();
			delete p;
			LOG(INFO) << "已删除对象,GetCurrentThreadId=" << GetCurrentThreadId();

		}
		g_space.Update();
	}
	accept.Stop();
	LOG(INFO) << "正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
	Sleep(3000);
	//system(0);
	return 0;
}
//*/