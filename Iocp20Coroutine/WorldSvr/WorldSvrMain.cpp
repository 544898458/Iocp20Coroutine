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

	g_CoDbPlayer.Init(threadPoolNetwork.GetIocp());
	
	Iocp::Server<WorldSvrAcceptGame> acceptGame(threadPoolNetwork.GetIocp());
	Iocp::Server<WorldSvrAcceptGate> acceptGate(threadPoolNetwork.GetIocp());
	Iocp::WsaStartup();
	acceptGame.Init<WorldSessionFromGame>(PORT_WORLDSVR_ACCEPT_GAME);
	acceptGate.Init<WorldSessionFromGate>(PORT_WORLDSVR_ACCEPT_GATE);
	
	while (g_running)
	{
		Sleep(100);
		acceptGame.m_Server.m_Sessions.Update([]() {});
		acceptGate.m_Server.m_Sessions.Update([]() {});
		CoTimer::Update();
		g_CoDbPlayer.Process();
	}
	acceptGame.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
