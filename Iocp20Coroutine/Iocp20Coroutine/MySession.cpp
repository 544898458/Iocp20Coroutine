#include <glog/logging.h>

#include "IocpNetwork/ServerTemplate.h"
#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "IocpNetwork/SessionSocketCompeletionKey.cpp"
#include "websocketfiles-master/src/ws_endpoint.cpp"
#include "./IocpNetwork/WebSocketSession.cpp"

#include "MySession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
#include "MsgQueue.h"
#include "Space.h"
#include "CoTimer.h"

//template<MySession>
//std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;


template bool Iocp::Server::Init<WebSocketSession<MySession> >();
template class Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >;


template<class T>
void MySession::Send(const T& ref)
{
	WebSocketPacket wspacket;
	// set FIN and opcode
	wspacket.set_fin(1);
	wspacket.set_opcode(0x02);// packet.get_opcode());

	std::stringstream buffer;
	msgpack::pack(buffer, ref);
	buffer.seekg(0);

	// deserialize the buffer into msgpack::object instance.
	std::string str(buffer.str());
	wspacket.set_payload(str.data(), str.size());
	ByteBuffer output;
	// pack a websocket data frame
	wspacket.pack_dataframe(output);
	//send to client
	//this->to_wire(output.bytes(), output.length());
	this->m_pWsSession->Send(output.bytes(), output.length());
}


CoTask<int> TraceEnemy(Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)
{
	KeepCancel kc(funCancel);
	bool stop = false;
	funCancel = [&stop]() {stop = true; };
	while (true)
	{
		if (co_await CoTimer::WaitNextUpdate(funCancel))
		{
			LOG(INFO) << "调用者手动取消了协程TraceEnemy";
			co_return 0;
		}
		if (stop)
		{

			LOG(INFO) << "TraceEnemy协程正常退出";
			co_return 0;
		}
		x -= 0.01;

		Broadcast<MsgNotifyPos,WebSocketSession<MySession>>(MsgNotifyPos(pEntity, x, z));
	}
}

MySession::MySession() : m_entity(5, g_space, TraceEnemy), m_msgQueue(this)
{
}
void MySession::OnRecvWsPack(const char buf[], const int len)
{
	msgpack::object_handle oh = msgpack::unpack(buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
	LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	auto pSessionSocketCompeletionKey = this->m_pWsSession->m_pSession;
	switch (msgId)
	{
	case MsgId::Login:
	{
		const auto msg = obj.as<MsgLogin>();
		pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
	}
	break;
	case MsgId::Move:
	{
		const auto msg = obj.as<MsgMove>();
		pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
	}
	break;
	}
}
inline void MySession::OnInit(WebSocketSession<MySession>* pWsSession)
{
	m_pWsSession = pWsSession;
	g_space.setEntity.insert(&m_entity);

}
void MySession::OnDestroy()
{
	g_space.setEntity.erase(&m_entity);
}
template void MySession::Send(const MsgLoginRet&);
template void MySession::Send(const MsgNotifyPos&);

template<class T,class T_Session>
void Broadcast(const T& msg)
{
	std::lock_guard lock(g_setSessionMutex<T_Session>);
	for (auto p : g_setSession<T_Session>)
	{
		p->Session.m_Session.Send(msg);
	}
}

template void Broadcast<MsgLoginRet,WebSocketSession<MySession>>(const MsgLoginRet&);
template void Broadcast<MsgNotifyPos, WebSocketSession<MySession>>(const MsgNotifyPos&);
template void Broadcast<MsgChangeSkeleAnim, WebSocketSession<MySession>>(const MsgChangeSkeleAnim&);
template class WebSocketSession<MySession>;
template class WebSocketEndpoint<MySession, Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> > >;