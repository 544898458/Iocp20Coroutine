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
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "WorldServer.h"
#include "WorldSession.h"


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

int main()
{
	Iocp::ThreadPool::Init();
	//Iocp::Server<WorldClient> accept(Iocp::ThreadPool::GetIocp());
	Iocp::Server<WorldServer> accept(Iocp::ThreadPool::GetIocp());
	accept.WsaStartup();
	accept.Init<WorldSession>(12346);
	//accept.Connect<WorldClientSession>( L"127.0.0.1", L"12346");
	while (g_running)
	{
		Sleep(100);
		//accept.m_Server.Update();
		CoTimer::Update();
	}
	accept.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
