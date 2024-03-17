#include "MsgQueue.h"

MsgId MsgQueue::PopMsg()
{
	std::lock_guard lock(this->m_mutex);
	if (this->m_queueMsgId.empty())
		return (MsgId)0;// MsgId::Invalid_0;

	const auto msgId = this->m_queueMsgId.front();
	this->m_queueMsgId.pop_front();

	return msgId;
}