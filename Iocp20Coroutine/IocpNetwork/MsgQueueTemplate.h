#pragma once

#include "MsgQueue.h"

template<class T_Msg>
void MsgQueue::Push(const T_Msg& msg, std::deque<T_Msg>& queue)
{
	std::lock_guard lock(this->m_mutex);
	queue.push_back(msg);
	this->m_queueMsgId.push_back(msg.id);
}

template<class T_Sub, class T_Msg>
void MsgQueue::OnRecv(std::deque<T_Msg>& queue, T_Sub& refSub, void (*funOnRecv)(T_Sub& refSub, const T_Msg&))
{
	std::lock_guard lock(this->m_mutex);
	const auto msg = queue.front();
	queue.pop_front();
	funOnRecv(refSub, msg);
}
