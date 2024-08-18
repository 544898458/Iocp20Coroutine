#include "MsgQueueMsgPack.h"
#include "../IocpNetwork/MsgQueueTemplate.h"
template<class T_Session>
template<typename T_Msg>
	requires requires(T_Session& session)
{
	session.GetQueue<T_Msg>();
}
void MsgQueueMsgPack<T_Session>::PushMsg(T_Session& session, const msgpack::object& obj)
{
	const auto msg = obj.as<T_Msg>();
	m_MsgQueue.Push(msg, session.GetQueue<T_Msg>());
}