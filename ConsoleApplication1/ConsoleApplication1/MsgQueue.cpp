#include "MsgQueue.h"

void MsgQueue::Process()
{
	MsgId msgId = MsgId::Login;
	{
		std::lock_guard(this->mutex);
		if (this->queueMsgId.empty())
			return;
		const auto msgId = this->queueMsgId.front();
		this->queueMsgId.pop_front();
	}
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

inline void MsgQueue::Push(const MsgLogin& msg)
{
	std::lock_guard(this->mutex);
	queueLogin.push_back(msg);
	queueMsgId.push_back(Login);
}

void MsgQueue::OnRecv(const MsgLogin& msg)
{

}
