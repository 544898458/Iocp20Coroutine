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
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include <memory>
std::unique_ptr<Iocp::SessionSocketCompletionKey<GameClientSession>> g_ConnectToGameSvr;
bool g_running(true);
void SendToGameSvr(const void* buf, const int len,uint64_t gateSessionId)
{
	g_ConnectToGameSvr->Session.Send(MsgGate转发(buf, len, gateSessionId));
}
std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
void SendToGateClient(const void* buf, const int len, uint64_t gateSessionId)
{
	//g_upGateSvr->m_Server.m_Sessions.Send(gateSessionId, buf, len));
	auto pSession = (GateSession*)gateSessionId;
	pSession->m_refSession.m_webSocketEndpoint->Send(buf, len); 
}
int main()
{
	Iocp::ThreadPool::Init();
	g_upGateSvr.reset(new Iocp::Server<GateServer>(Iocp::ThreadPool::GetIocp()));
	g_upGateSvr->WsaStartup();
	g_upGateSvr->Init<GateSession::CompeletionKeySession>(12348);

	g_ConnectToGameSvr.reset(Iocp::Client::Connect<GameClientSession>(L"127.0.0.1", L"12345", Iocp::ThreadPool::GetIocp()));


	while (g_running)
	{
		Sleep(100);
		g_upGateSvr->m_Server.m_Sessions.Update([]() {}); ;
		//CoTimer::Update();
	}
	g_upGateSvr->Stop();
	LOG(INFO) << "GateSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();

}