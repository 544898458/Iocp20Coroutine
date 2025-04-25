#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")
//
#include "../IocpNetwork/Server.h"
#include "../IocpNetwork/Client.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
//#include <glog/logging.h>
//#include "../IocpNetwork/StrConv.h"
#include "GateServer.h"
#include "ClientSession_GateToGame.h"
#include "../GameSvr/MyMsgQueue.h"
#include <memory>
#include "../LogStrategy/StrategyLog.h"
#include "ClientSession_GateToWorld.h"
#include "../IocpNetwork/WsaStartUp.h"
#include "../GameSvr/AllPort.h"
#include "../MiniDump/MiniDump.h"
#include "../IocpNetwork/SslTlsSvr.h"

#include <openssl/ssl.h>
#include <openssl/err.h>


std::unique_ptr<Iocp::SessionSocketCompletionKey<ClientSession_GateToGame>> g_ConnectToGameSvr;
bool g_running(true);
//void SendToGameSvr转发(const void* buf, const int len, uint64_t gateSessionId)// , uint32_t sn)
//{
//	g_ConnectToGameSvr->Session.Send(MsgGate转发(buf, len, gateSessionId, 0));
//}

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

template<>
void SendToGameSvr(const MsgGate转发& refMsg, const uint64_t gateSessionId, uint32_t snSend)
{
	refMsg.msg.sn = snSend;
	g_ConnectToGameSvr->Session.Send(refMsg);
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
template<class T>
void SendToGateClient(const T& refMsg, uint64_t gateSessionId)
{
	if (!g_upGateSvr->m_Server.m_Sessions.GetSession(gateSessionId, [&refMsg, gateSessionId](auto& refGateSession) {
		if (refGateSession.Session.m_Session.m_bLoginOk || refMsg.msg.id == Login)
		{
			refMsg.msg.sn = ++refGateSession.Session.m_Session.m_snSendToClient;
			MsgPack::SendMsgpack(refMsg, [gateSessionId, &refGateSession](const void* buf, int len)
				{
					refGateSession.Session.m_Session.m_refSession.Send(buf, len);
				}, false);
		}
		}))
	{
		//LOG(WARNING) << "无法发给游戏客户端，找不到GateSession，可能早已断线,gateSessionId=" << gateSessionId << ",id=" << refMsg.msg.id << ",sn=" << refMsg.msg.sn << ",rpcSnId=" << refMsg.msg.rpcSnId;
	}
}

template void SendToGateClient(const MsgGateSvr转发GameSvr消息给游戏前端& refMsg, uint64_t gateSessionId);
template void SendToGateClient(const MsgLoginResponce& refMsg, uint64_t gateSessionId);

void BroadToGateClient(const MsgGateSvr转发WorldSvr消息给游戏前端& refMsg)
{
	g_upGateSvr->m_Server.m_Sessions.Broadcast(refMsg);
}

int main()
{
	MiniDump::Install("GateSvr");
	//memset((void*)12341324, 23423, 234234);
	SslTlsSvr::InitAll();
	FLAGS_alsologtostderr = true;//是否将日志输出到文件和stderr
	FLAGS_colorlogtostdout = true;
	FLAGS_colorlogtostderr = true;//20240216
	google::InitGoogleLogging("GateSvr");//使用glog之前必须先初始化库，仅需执行一次，括号内为程序名

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
		CoTimer::Update();
	}
	g_upGateSvr->Stop();
	LOG(INFO) << "GateSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();

}