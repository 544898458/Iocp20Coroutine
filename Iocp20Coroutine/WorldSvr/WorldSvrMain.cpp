#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")

#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
#include <glog/logging.h>
#include <msgpack.hpp>
#include "../IocpNetwork/StrConv.h"
#include "WorldSvrAcceptGame.h"
#include "WorldSvrAcceptGate.h"
#include "WorldSessionFromGame.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "DbPlayer.h"
#include "../IocpNetwork/WsaStartUp.h"
#include "../GameSvr/AllPort.h"
#include "../MiniDump/MiniDump.h"
#include "../慢操作AliyunGreen/慢操作AliyunGreen.h"

BOOL g_running = TRUE;
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT://控制台关闭（点右上角X关闭）
		LOG(WARNING) << "不能点右上角X关闭，可能有数据没保存";
		g_running = FALSE;
		Sleep(3000);
		_CrtDumpMemoryLeaks();	 //显示内存泄漏报告
		break;
	case CTRL_C_EVENT:
		g_running = FALSE;
		break;
	}
	return TRUE;
}

CoDb<DbPlayer> g_CoDbPlayer;
慢操作AliyunGreen g_慢操作AliyunGreen;

std::unique_ptr<Iocp::Server<WorldSvrAcceptGate>> g_upAcceptGate;
template<typename T>
void BroadcastToGate(const T& refMsg)
{
	g_upAcceptGate->m_Server.m_Sessions.Broadcast(refMsg);
}
template void BroadcastToGate(const Msg在线人数& refMsg);
/*
架构

多个 GameSvr 一一→ 一个WorldSvr对应一个DB区

多个 GateSvr ____↗

多个 游戏客户端


登录验证逻辑放哪里，
GameSvr,GateSvr,WorldSvr都可以做，
但是GameSvr是计算密集，功能越少越好
所以只能在Gate和World选一个
因为Gate有多个，同号踢人还要依赖World，所以现在放到WorldSvr
*/
int main()
{
	MiniDump::Install("WorldSvr");
	//memset((void*)12341324, 23423, 234234);

	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216
	google::InitGoogleLogging("WorldSvr");//使用glog之前必须先初始化库，仅需执行一次，括号内为程序名

	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();

	Iocp::ThreadPool threadPoolDb;
	threadPoolDb.Init();

	g_CoDbPlayer.m_慢操作.Init(threadPoolNetwork.GetIocp());
	g_慢操作AliyunGreen.m_慢操作.Init(threadPoolNetwork.GetIocp());

	Iocp::Server<WorldSvrAcceptGame> acceptGame(threadPoolNetwork.GetIocp());
	g_upAcceptGate.reset(new Iocp::Server<WorldSvrAcceptGate>(threadPoolNetwork.GetIocp()));
	Iocp::WsaStartup();
	acceptGame.Init<WorldSessionFromGame>(PORT_WORLDSVR_ACCEPT_GAME);
	g_upAcceptGate->Init<WorldSessionFromGate>(PORT_WORLDSVR_ACCEPT_GATE);
	
	while (g_running)
	{
		Sleep(100);
		acceptGame.m_Server.m_Sessions.Update([]() {});
		g_upAcceptGate->m_Server.m_Sessions.Update([]() {});
		CoTimer::Update();
		g_CoDbPlayer.m_慢操作.Process();
		g_慢操作AliyunGreen.m_慢操作.Process();
	}
	acceptGame.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
