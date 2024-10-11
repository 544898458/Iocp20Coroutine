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
#include "ClientSession_GateToGame.h"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include <memory>
#include "../LogStrategy/StrategyLog.h"
#include "ClientSession_GateToWorld.h"
#include "../IocpNetwork/WsaStartUp.h"
#include "../Iocp20Coroutine/AllPort.h"

std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>> g_ConnectToGameSvr;
bool g_running(true);
void SendToGameSvr转发(const void* buf, const int len, uint64_t gateSessionId)// , uint32_t sn)
{
	static uint32_t sn = 0;
	++sn;
	g_ConnectToGameSvr->Session.Send(MsgGate转发(buf, len, gateSessionId, sn));
}

/// <summary>
/// 不需要转发的
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="refMsg"></param>
template<class T>
void SendToGameSvr(const T& refMsg, const uint64_t gateSessionId, uint32_t snSend)
{
	MsgPack::SendMsgpack(refMsg, [gateSessionId, snSend](const void* buf, int len)
		{
			g_ConnectToGameSvr->Session.Send(MsgGate转发(buf, len, gateSessionId, snSend));
		}, false);
}
template void SendToGameSvr(const MsgGateDeleteSession&, const uint64_t gateSessionId, uint32_t);
template void SendToGameSvr(const MsgGateAddSession&, const uint64_t gateSessionId, uint32_t);

std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>> g_ConnectToWorldSvr;

template<class T>
void SendToWorldSvr转发(const T& refMsg, const uint64_t gateSessionId)
{
	MsgPack::SendMsgpack(refMsg, [gateSessionId](const void* buf, int len)
		{
			static uint32_t sn = 0;
			++sn;
			g_ConnectToWorldSvr->Session.Send(MsgGate转发(buf, len, gateSessionId, sn));
		}, false);
}
template void SendToWorldSvr转发(const MsgLogin&, const uint64_t gateSessionId);
template void SendToWorldSvr转发(const MsgGateDeleteSession&, const uint64_t gateSessionId);
template void SendToWorldSvr转发(const MsgGateDeleteSessionResponce&, const uint64_t gateSessionId);


std::unique_ptr<Iocp::Server<GateServer>> g_upGateSvr;
void SendToGateClient(const void* buf, const int len, uint64_t gateSessionId)
{
	//{
	//	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	//	msgpack::object obj = oh.get();
	//	const auto msg = MsgHead::GetMsgId(obj);
	//	//LOG(INFO) << obj;
	//	if (msg.id == MsgId::AddRoleRet)
	//	{
	//		static int n = 0;
	//		++n;
	//		LOG(INFO) << "AddRoleRet:" << n;
	//	}
	//}

	auto pSession = g_upGateSvr->m_Server.m_Sessions.GetSession(gateSessionId);
	//auto pSession = (GateSession*)gateSessionId;
	CHECK_NOTNULL_VOID(pSession);
	if (pSession->Session.m_Session.m_bLoginOk)
	{
		pSession->Session.m_Session.m_refSession.Send(buf, len);
	}
}

int main()
{
	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216

	Iocp::ThreadPool threadPoolNetwork;
	threadPoolNetwork.Init();
	g_upGateSvr.reset(new Iocp::Server<GateServer>(threadPoolNetwork.GetIocp()));
	Iocp::WsaStartup();
	g_upGateSvr->Init<GateSession::CompeletionKeySession>(PORT_GATESVR);

	g_ConnectToGameSvr.reset(Iocp::Client::Connect<ClientSession_GateToGame>(L"127.0.0.1", PORT_GAMESVR, threadPoolNetwork.GetIocp()));
	g_ConnectToWorldSvr.reset(Iocp::Client::Connect<ClientSession_GateToWorld>(L"127.0.0.1", PORT_WORLDSVR_ACCEPT_GATE, threadPoolNetwork.GetIocp()));

	while (g_running)
	{
		Sleep(100);
		g_upGateSvr->m_Server.m_Sessions.Update([]() {});
		g_ConnectToWorldSvr->Session.Process();
		//CoTimer::Update();
	}
	g_upGateSvr->Stop();
	LOG(INFO) << "GateSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();

}