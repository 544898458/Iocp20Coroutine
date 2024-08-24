#include "GateSession.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

template class Iocp::SessionSocketCompletionKey<GateSession>;
template class MsgQueueMsgPack<GateSession>;

int GateSession::OnRecv(CompeletionKeySession&, const void* buf, int len)
{
    return 0;
}

void GateSession::OnDestroy()
{
}

void GateSession::OnInit(CompeletionKeySession& refSession, GateServer&)
{
}
