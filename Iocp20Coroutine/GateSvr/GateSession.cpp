#include "GateSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "GateServer.h"
#include "../IocpNetwork/SessionsTemplate.h"

template class Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>;
template class MsgQueueMsgPack<GateSession::CompeletionKeySession>;
template class WebSocketSession<GateSession>;
template class WebSocketEndpoint<GateSession, Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession> >;
template void WebSocketSession<GateSession>::OnInit<GateServer>(Iocp::SessionSocketCompletionKey<GateSession::CompeletionKeySession>& refSession, GateServer& server);
//int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
//{
//    return 0;
//}

void SendToGameSvr(const void* buf, const int len, uint64_t gateSessionId);
void GateSession::OnRecvWsPack(const void* buf, const int len)
{
	SendToGameSvr(buf, len, (uint64_t)this);
}

template<class T>
void SendToGameSvr(const T& refMsg);

void GateSession::OnDestroy()
{
	SendToGameSvr<MsgGateDeleteSession>({ .gateClientSessionId = (uint64_t)this });
}

void GateSession::OnInit(CompeletionKeySession& refSession, GateServer& refServer)
{
	refServer.m_Sessions.AddSession(refSession.m_pSession, [this, &refSession, &refServer]()
		{
			LOG(INFO) << "游戏客户端已连上";
			//m_pServer = &refServer;
			//m_pSession = &refSession;

			SendToGameSvr<MsgGateAddSession>({ .gateClientSessionId = (uint64_t)this });
		},(uint64_t)this);

}

void GateSession::Process()
{
}
