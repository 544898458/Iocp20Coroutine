#define _CRT_SECURE_NO_WARNINGS

#include <vld.h>
#include <string.h>
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
#include "Space.h"
#include "Entity.h"
#include "MyServer.h"
#include "MySession.h"
#include <glog/logging.h>
#include "WorldClient.h"
#include "../IocpNetwork/MsgPack.h"
#include "AiCo.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#pragma comment(lib, "glog.lib")

BOOL g_running = TRUE;
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

		g_running = FALSE;

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
		g_running = FALSE;
		//return false;
		break;
	}
	return TRUE;
}

std::unique_ptr<Iocp::Server<WorldClient> > g_worldSvr;// (Iocp::ThreadPool::GetIocp());
std::unique_ptr<Iocp::SessionSocketCompeletionKey<WorldClientSession>> g_ConnectToWorldSvr;
void SendToWorldSvr(const MsgSay& msg)
{
	//g_worldSvr->m_Server.m_Sessions.Broadcast(msg);
	//g_worldSvr->m_pClientSession->Send(msg);
	MsgPack::SendMsgpack(msg, [](const void* buf, int len) {g_ConnectToWorldSvr->Send(buf, len); });
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
	LOG(INFO) << "GameSvr已启动";

	SetConsoleCtrlHandler(fun, TRUE);
	Iocp::ThreadPool::Init();
	Iocp::Server<MyServer> accept(Iocp::ThreadPool::GetIocp());
	//g_worldSvr.reset( new Iocp::Server<WorldServer>(Iocp::ThreadPool::GetIocp()) );

	Iocp::Server<MyServer>::WsaStartup();
	accept.Init<WebSocketSession<MySession>>(12345);
	//Iocp::ThreadPool::Add(accept.GetIocp());

	//g_worldSvr->Init<WorldSession>(12346);
	g_worldSvr.reset(new Iocp::Server<WorldClient>(Iocp::ThreadPool::GetIocp()));
	g_ConnectToWorldSvr.reset( g_worldSvr->Connect<WorldClientSession>(L"127.0.0.1", L"12346") );
	WorldClient::m_funBroadcast = [&accept](const MsgSay& msg) {accept.m_Server.m_Sessions.Broadcast(msg); };

	//Iocp::ThreadPool::Add(g_worldSvr.GetIocp());


	Entity entityMonster;
	entityMonster.Init(-5, accept.m_Server.m_space, "altman-red");
	entityMonster.m_nickName = "怪";
	accept.m_Server.m_space.setEntity.insert(&entityMonster);

	//主逻辑工作线程
	while (g_running)
	{
		Sleep(100);
		accept.m_Server.Update();
		CoTimer::Update();
	}
	accept.Stop();
	LOG(INFO) << "正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
	Sleep(3000);
	//system(0);
	return 0;
}
//*/