#include "pch.h"
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
#include "../LogStrategy/StrategyLog.h"

std::unique_ptr<Iocp::SessionSocketCompletionKey<GameClientSession>> g_ConnectToGameSvr;
bool g_running(true);
void SendToGameSvr(const void* buf, const int len, uint64_t gateSessionId)// , uint32_t sn)
{
	static uint32_t sn = 0;
	++sn;
	g_ConnectToGameSvr->Session.Send(MsgGate转发(buf, len, gateSessionId, sn));
}

template<class T>
void SendToGameSvr(const T& refMsg)//, uint32_t snSend)
{
	//refMsg.msg.SetSn(snSend);
	g_ConnectToGameSvr->Session.Send(refMsg);
}
template void SendToGameSvr(const MsgGateDeleteSession&);// , uint32_t);
template void SendToGameSvr(const MsgGateAddSession&);// , uint32_t);

std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
void SendToGateClient(const void* buf, const int len, uint64_t gateSessionId)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msg = Msg::GetMsgId(obj);
	//LOG(INFO) << obj;
	if (msg.id == MsgId::AddRoleRet)
	{
		static int n = 0;
		++n;
		//LOG(INFO) << "AddRoleRet:" << n;
	}

	auto pSession = g_upGateSvr->m_Server.m_Sessions.GetSession(gateSessionId);
	//auto pSession = (GateSession*)gateSessionId;
	CHECK_NOTNULL_VOID(pSession);
	pSession->Session.m_Session.m_refSession.Send(buf, len);
}
int main()
{
	Iocp::ThreadPool threadPoolNetwork; 
	threadPoolNetwork.Init();
	g_upGateSvr.reset(new Iocp::Server<GateServer>(threadPoolNetwork.GetIocp()));
	g_upGateSvr->WsaStartup();
	g_upGateSvr->Init<GateSession::CompeletionKeySession>(12348);

	g_ConnectToGameSvr.reset(Iocp::Client::Connect<GameClientSession>(L"127.0.0.1", L"12345", threadPoolNetwork.GetIocp()));


	while (g_running)
	{
		Sleep(100);
		g_upGateSvr->m_Server.m_Sessions.Update([]() {}); ;
		//CoTimer::Update();
	}
	g_upGateSvr->Stop();
	LOG(INFO) << "GateSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();

}