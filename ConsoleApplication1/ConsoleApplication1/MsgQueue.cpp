#include "MsgQueue.h"
MsgQueue g_MsgQueue;

void MsgQueue::Process()
{
	if (this->queueMsgId.empty())
		return;
	const auto msgId = this->queueMsgId.front();
	this->queueMsgId.pop_front();
	switch (msgId)
	{
	case Login:
	{
		const auto msg = this->queueLogin.front();
		this->queueLogin.pop_front();
		OnRecv(msg);
	}
		break;
	default:
		break;
	}
}
