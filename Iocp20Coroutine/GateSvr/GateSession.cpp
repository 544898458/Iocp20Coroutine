#include "GateSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"


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

void GateSession::OnDestroy()
{
}

void GateSession::OnInit(CompeletionKeySession& refSession, GateServer&)
{
}

void GateSession::Process()
{
}
