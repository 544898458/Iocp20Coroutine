#include "pch.h"
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <vld.h>
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/Client.h"
#include "ClientSession_GameToWorld.h"
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <signal.h>
#include "../IocpNetwork/ServerEpoll.h"
#include "../IocpNetwork/ClientEpoll.h"
#include "ClientSession_GameToWorldEpoll.h"
#endif
#include <string.h>
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
#include "Space.h"
#include "Entity.h"
#include "GameSvr.h"
#include "GameSvrSession.h"
#include <glog/logging.h>
#include "../IocpNetwork/MsgPack.h"
#include "AiCo.h"
#include "PlayerGateSession_Game.h"
#include "../IocpNetwork/WsaStartUp.h"
#include "AllPort.h"
#include "../MiniDump/MiniDump.h"
#include "单位.h"
#include "枚举/战局类型.h"
#include "../读Yaml配置/翻译.h"
#include "../读Yaml配置/读GameSvr配置.h"


BOOL g_running = TRUE;

#ifdef _WIN32
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		LOG(WARNING) << "不能点右上角X关闭，可能有数据没保存";
		g_running = FALSE;
		Sleep(300000*1000);
		_CrtDumpMemoryLeaks();
		break;
	case CTRL_C_EVENT:
		g_running = FALSE;
		break;
	}
	return TRUE;
}
#else
void SignalHandler(int sig)
{
	LOG(INFO) << "收到信号:" << sig << ",开始关闭服务器";
	g_running = false;
}
#endif

std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld> > g_ConnectToWorldSvr;
std::unique_ptr<Iocp::Server<GameSvr> > g_upAccept;

template<class T>
void SendToWorldSvr(const T& refMsg)
{
	refMsg.msg.sn = ++g_ConnectToWorldSvr->m_snSend;
	MsgPack::SendMsgpack(refMsg, [](const void* buf, int len)
		{
			g_ConnectToWorldSvr->Send(buf, len);
		});	
}
template<class T>
void SendToWorldSvr转发(const T& refMsg, const uint64_t idGateSession)
{
	MsgPack::SendMsgpack(refMsg, [idGateSession](const void* buf, int len)
		{
			MsgGate转发 msg(buf, len, idGateSession, 0);
			SendToWorldSvr(msg);
		}, false);
}
template void SendToWorldSvr转发(const MsgSay& msg, const uint64_t idGateSession);
template void SendToWorldSvr转发(const MsgChangeMoney& msg, const uint64_t idGateSession);
template void SendToWorldSvr(const MsgSay& msg);
template void SendToWorldSvr(const MsgChangeMoney& msg);
template void SendToWorldSvr(const Msg战局结束& msg);
template void SendToWorldSvr(const Msg击杀& msg);

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string &refStrNickName)
{
	std::weak_ptr<PlayerGateSession_Game> spRet;
	g_upAccept->m_Server.m_Sessions.ForEach([&spRet, &refStrNickName](GameSvrSession& refSession) {
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

int main(int argc, char* argv[])
{
	const int gameSvrId = (argc >= 2) ? atoi(argv[1]) : 0;

#ifdef _WIN32
	MiniDump::Install("GameSvr");
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, false);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
	SetConsoleCtrlHandler(fun, TRUE);
#else
	signal(SIGTERM, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGPIPE, SIG_IGN);
#endif

	auto* pMemoryLeak = malloc(123);
	const char sz[] = "Memory Leak Test.Nei Cun Xie Lou Jian Ce.This is not a real Defect.Zhe Bu Shi Yi Ge Zhen De Que Xian.";
	memcpy(pMemoryLeak, sz, sizeof(sz));
	pMemoryLeak = nullptr;

	FLAGS_alsologtostderr = true;
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;
	FLAGS_log_dir = "./Log/GameSvr";
	google::InitGoogleLogging("GameSvr");

	LOG(INFO) << "Test GLOG_INFO";
	LOG(WARNING) << "Test GLOG_WARNING";
	LOG(ERROR) << "Test GLOG_ERROR";
	LOG(INFO) << "GameSvr已启动 GameSvrId=" << gameSvrId;
	单位::读配置文件();
	CHECK_RET_DEFAULT(读Yaml配置::读翻译配置文件("配置/翻译.yaml"));
	CHECK_RET_DEFAULT(读Yaml配置::读GameSvr配置文件("配置/GameSvr.yaml"));	

	读Yaml配置::GameSvr配置 gameSvrConfig;
	CHECK_RET_DEFAULT(读Yaml配置::FindGameSvr配置(gameSvrId, gameSvrConfig));

	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();

	g_upAccept.reset(new Iocp::Server<GameSvr>(threadPoolNetwork.GetIocp()));

	Iocp::WsaStartup();
	g_upAccept->Init<GameSvrSession>(gameSvrConfig.u16端口);

	g_ConnectToWorldSvr.reset(Iocp::Client::Connect<ClientSession_GameToWorld>("127.0.0.1", PORT_WORLDSVR_ACCEPT_GAME, threadPoolNetwork.GetIocp()));
	extern std::function<void(MsgSay const&)> m_funBroadcast;
	m_funBroadcast = [](const MsgSay& msg) {g_upAccept->m_Server.m_Sessions.Broadcast(msg); };

	FunCancel funCancelSpawnMonster;
	for (auto& spaceConfig : gameSvrConfig.vecSpaces)
	{
		if( 战局类型::单人ID_非法_MIN == spaceConfig.战局)
			continue;

		auto wpSpace = Space::AddSpace(spaceConfig.战局);
		CHECK_WP_RET_DEFAULT(wpSpace);
		AiCo::多人联机地图(*wpSpace.lock(), (uint16_t)spaceConfig.f半径, spaceConfig.u最大怪物数, funCancelSpawnMonster).RunNew();
	}

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
		CoTaskBool::Process();
	}

	g_upAccept->m_Server.OnAppExit();
	g_ConnectToWorldSvr->Session.OnAppExit();
	Space::StaticSave();
	CoTimer::OnAppExit();
	CoTask<int>::OnAppExit();

	if (funCancelSpawnMonster)
		funCancelSpawnMonster();

	g_upAccept->Stop();
	LOG(INFO) << "正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
	std::this_thread::sleep_for(std::chrono::seconds(3));
	return 0;
}
