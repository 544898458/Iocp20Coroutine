#pragma once

#include "MsgQueue.h"

/// <summary>
/// �����̣߳����߳�
/// </summary>
/// <typeparam name="T_Msg"></typeparam>
/// <param name="msg"></param>
/// <param name="queue"></param>
template<class T_Msg>
void MsgQueue::Push(const T_Msg& msg, std::deque<T_Msg>& queue)
{
	std::lock_guard lock(this->m_mutex);
	queue.push_back(msg);
	this->m_queueMsgId.push_back(msg.msg.id);
}

/// <summary>
/// ���̣߳����߳�
/// </summary>
/// <typeparam name="T_Sub"></typeparam>
/// <typeparam name="T_Msg"></typeparam>
/// <param name="queue"></param>
/// <param name="refSub"></param>
/// <param name="funOnRecv"></param>
template<class T_Sub, class T_Msg>
void MsgQueue::OnRecv(std::deque<T_Msg>& queue, T_Sub& refSub, void (T_Sub::*funOnRecv)(const T_Msg&))
{
	std::lock_guard lock(this->m_mutex);
	const auto msg = queue.front();
	queue.pop_front();
	++refSub.m_snRecv;
	assert(refSub.m_snRecv == msg.msg.sn);
	(refSub.*funOnRecv)(msg);
}
