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
#include "WorldServer.h"
#include "WorldSession.h"
#include "../CoRoutine/CoDbTemplate.h"

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

CoDb<DbTest> g_TestSave;
DbTest dt;
FunCancel fc;

CoTask<int> TestCoDb() 
{
	co_await g_TestSave.Save(dt, fc);
	co_return 0;
}
int main()
{
	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();
	Iocp::ThreadPool threadPoolDb;
	threadPoolDb.Init();
	g_TestSave.Init(threadPoolNetwork.GetIocp());
	//Iocp::Server<WorldClient> accept(Iocp::ThreadPool::GetIocp());
	Iocp::Server<WorldServer> accept(threadPoolNetwork.GetIocp());
	accept.WsaStartup();
	accept.Init<WorldSession>(12346);
	//accept.Connect<WorldClientSession>( L"127.0.0.1", L"12346");
	auto co = TestCoDb();
	co.Run();
	while (g_running)
	{
		Sleep(100);
		accept.m_Server.m_Sessions.Update([]() {});
		CoTimer::Update();
		g_TestSave.Process();
	}
	accept.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
