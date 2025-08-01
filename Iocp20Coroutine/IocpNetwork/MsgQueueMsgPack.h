#pragma once
#include "MsgQueue.h"
#include <msgpack.hpp>
template<class T_Session>
class MsgQueueMsgPack
{
public:
	MsgId PopMsg() { return m_MsgQueue.PopMsg(); }
	template<typename T_Msg>
		requires requires(T_Session& session)
	{
		session.GetQueue<T_Msg>();
	}
	void PushMsg(T_Session& session, const msgpack::object& obj);
	template<typename T_Msg>
		requires requires(T_Session& session)
	{
		session.GetQueue<T_Msg>();
	}
	void PushMsg(T_Session& session, const T_Msg& msg);
	template<class T_Msg>
	bool OnRecv(std::deque<T_Msg>& queue, T_Session& session, void (T_Session::* funOnRecv)(const T_Msg&))
	{
		return m_MsgQueue.OnRecv(queue, session, funOnRecv);
	}
	template<class T_Msg>
	bool OnRecv不处理序号(std::deque<T_Msg>& queue, T_Session& session, void (T_Session::* funOnRecv)(const T_Msg&))
	{
		return m_MsgQueue.OnRecv不处理序号(queue, session, funOnRecv);
	}
private:
	MsgQueue m_MsgQueue;
};

