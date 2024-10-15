#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS

#include <vld.h>
#include <string.h>
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
#include "Space.h"
#include "Entity.h"
#include "GameSvr.h"
#include "GameSvrSession.h"
#include <glog/logging.h>
#include "ClientSession_GameToWorld.h"
#include "../IocpNetwork/MsgPack.h"
#include "AiCo.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/WsaStartUp.h"
#include "AllPort.h"

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

//std::unique_ptr<Iocp::Server<WorldClient> > g_worldSvr;// (Iocp::ThreadPool::GetIocp());
std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>> g_ConnectToWorldSvr;

//void SendToWorldSvr(const MsgSay& msg){MsgPack::SendMsgpack(msg, [](const void* buf, int len) {g_ConnectToWorldSvr->Send(buf, len); });}
//void SendToWorldSvr(const MsgChangeMoney& msg) 
//{
//	msg.msg.sn = (++g_ConnectToWorldSvr->m_snSend);
//	MsgPack::SendMsgpack(msg, [](const void* buf, int len) {g_ConnectToWorldSvr->Send(buf, len); }); 
//}
template<class T>
void SendToWorldSvr(const T& refMsg, const uint64_t idGateSession)
{
	//msg.msg.sn = (++g_ConnectToWorldSvr->m_snSend);
	//MsgPack::SendMsgpack(msg, [](const void* buf, int len) {g_ConnectToWorldSvr->Send(buf, len); }); 

	//MsgPack::SendMsgpack(refMsg, [idGateSession](const void* buf, int len)
	//	{
	//		g_ConnectToWorldSvr->Session.Send(MsgGate转发(buf, len, idGateSession, ++g_ConnectToWorldSvr->m_snSend));
	//	}, false);


	MsgPack::SendMsgpack(refMsg, [idGateSession](const void* buf, int len)
		{
			MsgGate转发 msg(buf, len, idGateSession, ++g_ConnectToWorldSvr->m_snSend);
			MsgPack::SendMsgpack(msg, [](const void* buf转发, int len转发)
				{
					g_ConnectToWorldSvr->Send(buf转发, len转发);
				});
		}, false);
}
template void SendToWorldSvr(const MsgSay& msg, const uint64_t idGateSession);
template void SendToWorldSvr(const MsgChangeMoney& msg, const uint64_t idGateSession);

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
	memcpy(pMemoryLeak, sz, sizeof(sz));
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
	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();

	Iocp::Server<GameSvr> accept(threadPoolNetwork.GetIocp());
	//g_worldSvr.reset( new Iocp::Server<WorldServer>(Iocp::ThreadPool::GetIocp()) );

	Iocp::WsaStartup();
	accept.Init<GameSvrSession>(PORT_GAMESVR);
	//Iocp::ThreadPool::Add(accept.GetIocp());

	//g_worldSvr->Init<WorldSession>(12346);
	//g_worldSvr.reset(new Iocp::Server<WorldClient>(Iocp::ThreadPool::GetIocp()));
	g_ConnectToWorldSvr.reset(Iocp::Client::Connect<ClientSession_GameToWorld>(L"127.0.0.1", PORT_WORLDSVR_ACCEPT_GAME, threadPoolNetwork.GetIocp()));
	extern std::function<void(MsgSay const&)> m_funBroadcast;
	m_funBroadcast = [&accept](const MsgSay& msg) {accept.m_Server.m_Sessions.Broadcast(msg); };


	FunCancel funCancelSpawnMonster;
	AiCo::SpawnMonster(accept.m_Server.m_Space无限刷怪, funCancelSpawnMonster).RunNew();
	
	//主逻辑工作线程
	using namespace std;
	std::chrono::system_clock::time_point timeLast = std::chrono::system_clock::now();

	std::chrono::milliseconds msSleep = 100ms;
	const std::chrono::milliseconds ms10Frame = 1s;
	int i = 0;
	while (g_running)
	{
		++i;
		if (i >= 10)
		{
			i = 0;
			auto now = std::chrono::system_clock::now();
			std::chrono::duration duration = now - timeLast;
			timeLast = now;
			if (duration > ms10Frame)
			{
				if (msSleep > 0ms)
					--msSleep;
			}
			else
				++msSleep;
		}
		if (msSleep > 0ms)
			std::this_thread::sleep_for(msSleep);

		accept.m_Server.Update();
		g_ConnectToWorldSvr->Session.Process();
		CoTimer::Update();
		CoTask<int>::Process();
		void CrowToolUpdate();
		CrowToolUpdate();
	}

	if (funCancelSpawnMonster)
		funCancelSpawnMonster();

	accept.Stop();
	LOG(INFO) << "正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
	Sleep(3000);
	//system(0);
	return 0;
}
//*/