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
#include "../MiniDump/MiniDump.h"
#include "单位.h"

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
		Sleep(300000*1000);
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
std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld> > g_ConnectToWorldSvr;
std::unique_ptr<Iocp::Server<GameSvr> > g_upAccept;
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
template void SendToWorldSvr(const Msg战局结束& msg, const uint64_t idGateSession);

LONG WINAPI UnhandledExceptionFilter_SpawDmp(struct _EXCEPTION_POINTERS* ExceptionInfo);

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string &refStrNickName)
{
	std::weak_ptr<PlayerGateSession_Game> spRet;
	g_upAccept->m_Server.m_Sessions.ForEach([&spRet, &refStrNickName](GameSvrSession& refSession) {
		//CHECK_NOTNULL_VOID(pSession);
		auto &refMap = refSession.m_mapPlayerGateSession;
		auto iterFind = std::find_if(refMap.begin(), refMap.end(),
		[&refStrNickName](const auto& pair)->bool
		{
			return pair.second->NickName() == refStrNickName;
		});
		if (refMap.end() == iterFind)
			return;

		spRet = iterFind->second;
	});

	return spRet;
}
int main(void)
{
	MiniDump::Install("GameSvr");
	//memset((void*)12341324, 23423, 234234);
	//屏蔽控制台最小按钮和关闭按钮
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, false);
	//RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);

	auto* pMemoryLeak = malloc(123);//测试内存泄漏
	const char sz[] = "Memory Leak Test.Nei Cun Xie Lou Jian Ce.This is not a real Defect.Zhe Bu Shi Yi Ge Zhen De Que Xian.";
	memcpy(pMemoryLeak, sz, sizeof(sz));
	pMemoryLeak = nullptr;

	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216
	google::InitGoogleLogging("GameSvr");//使用glog之前必须先初始化库，仅需执行一次，括号内为程序名

	LOG(INFO) << "Test GLOG_INFO";
	LOG(WARNING) << "Test GLOG_WARNING";
	LOG(ERROR) << "Test GLOG_ERROR";
	LOG(INFO) << "GameSvr已启动";
	单位::读配置文件();

	SetConsoleCtrlHandler(fun, TRUE);
	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();

	g_upAccept.reset(new Iocp::Server<GameSvr>(threadPoolNetwork.GetIocp()));
	//g_worldSvr.reset( new Iocp::Server<WorldServer>(Iocp::ThreadPool::GetIocp()) );

	Iocp::WsaStartup();
	g_upAccept->Init<GameSvrSession>(PORT_GAMESVR);
	//Iocp::ThreadPool::Add(accept.GetIocp());

	//g_worldSvr->Init<WorldSession>(12346);
	//g_worldSvr.reset(new Iocp::Server<WorldClient>(Iocp::ThreadPool::GetIocp()));
	g_ConnectToWorldSvr.reset(Iocp::Client::Connect<ClientSession_GameToWorld>(L"127.0.0.1", PORT_WORLDSVR_ACCEPT_GAME, threadPoolNetwork.GetIocp()));
	extern std::function<void(MsgSay const&)> m_funBroadcast;
	m_funBroadcast = [](const MsgSay& msg) {g_upAccept->m_Server.m_Sessions.Broadcast(msg); };

	auto wpSpace无限刷怪 = Space::AddSpace(战局类型::多玩家混战);
	CHECK_WP_RET_DEFAULT(wpSpace无限刷怪);

	FunCancel funCancelSpawnMonster;
	AiCo::多人联机地图(*wpSpace无限刷怪.lock(), funCancelSpawnMonster).RunNew();

	//主逻辑工作线程
	using namespace std;
	std::chrono::system_clock::time_point timeLast = std::chrono::system_clock::now();

	const std::chrono::system_clock::duration msSleep目标 = 100ms;
	std::chrono::system_clock::duration msSleep = msSleep目标;
	const uint8_t u8Frames = 10;
	const std::chrono::system_clock::duration ms10Frame = msSleep * u8Frames;
	const std::chrono::system_clock::duration ms10Frame_1X1 = ms10Frame + msSleep * (u8Frames / 10);
	const std::chrono::system_clock::duration ms10Frame_0X9 = ms10Frame - msSleep * (u8Frames / 10);
	int i = 0;
	while (g_running)
	{
		++i;
		if (i >= u8Frames)
		{
			i = 0;
			auto now = std::chrono::system_clock::now();
			std::chrono::duration duration = now - timeLast;
			timeLast = now;
			if (duration > ms10Frame_1X1)
			{
				msSleep -= std::min(msSleep, (duration - ms10Frame) / u8Frames);
			}
			else if (duration < ms10Frame_0X9)
			{
				msSleep += std::min(msSleep目标, (ms10Frame - duration) / u8Frames);
			}
		}
		if (msSleep > 0ms)
			std::this_thread::sleep_for(msSleep);

		g_upAccept->m_Server.Update();
		g_ConnectToWorldSvr->Session.Process();
		Space::StaticUpdate();
		CoTimer::Update();
		CoTask<int>::Process();
	}

	g_upAccept->m_Server.OnAppExit();
	g_ConnectToWorldSvr->Session.OnAppExit();
	Space::StaticOnAppExit();
	CoTimer::OnAppExit();
	CoTask<int>::OnAppExit();

	if (funCancelSpawnMonster)
		funCancelSpawnMonster();

	g_upAccept->Stop();
	LOG(INFO) << "正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
	Sleep(3000);
	//system(0);
	return 0;
}