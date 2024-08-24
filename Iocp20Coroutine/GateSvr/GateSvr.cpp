//#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")
//
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/Client.h"
#include "../IocpNetwork/ThreadPool.h"
//#include "../CoRoutine/CoTimer.h"
//#include <glog/logging.h>
//#include "../IocpNetwork/StrConv.h"
#include "GateServer.h"
#include "GameClientSession.h"

std::unique_ptr<Iocp::SessionSocketCompletionKey<GameClientSession>> g_ConnectToGameSvr;

int main()
{
	Iocp::ThreadPool::Init();
	Iocp::Server<GateServer> accept(Iocp::ThreadPool::GetIocp());
	accept.WsaStartup();
	accept.Init<GateSession>(12348);

	g_ConnectToGameSvr.reset(Iocp::Client::Connect<GameClientSession>(L"127.0.0.1", L"12345", Iocp::ThreadPool::GetIocp()));


	//while (g_running)
	//{
	//	Sleep(100);
	//	accept.m_Server.m_Sessions.Update([]() {}); ;
	//	CoTimer::Update();
	//}
	//accept.Stop();
	//LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();

}