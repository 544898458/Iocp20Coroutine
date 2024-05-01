#include <glog/logging.h>

//#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "SessionsTemplate.h"
#include "MySession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
//#include "MsgQueue.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
//template<MySession>
//std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;
template Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >;


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
		x -= 0.01f;

		pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z));
	}
}

MySession::MySession() : m_msgQueue(this)
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
void MySession::OnInit(WebSocketSession<MySession>* pWsSession, MyServer& server)
{
	server.m_Sessions.AddSession(pWsSession->m_pSession, [this, pWsSession, &server]()
		{
			m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
			m_pServer->m_space.setEntity.insert(&m_entity);
			m_pServer = &server;
			m_pWsSession = pWsSession;
		});
	/*
	std::lock_guard lock(m_pServer->m_Sessions.m_setSessionMutex);

	m_entity.Init(5, m_pServer->m_space, TraceEnemy, this);
	m_pWsSession = pWsSession;
	m_pServer->m_space.setEntity.insert(&m_entity);
	m_pServer->m_Sessions.m_setSession.insert(pWsSession->m_pSession);
	//#include <glog/logging.h>
	LOG(INFO) << "添加Session，剩余" << m_pServer->m_Sessions.m_setSession.size();
	*/

}
void MySession::OnDestroy()
{
	m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
		{
			m_pServer->m_space.setEntity.erase(&m_entity);
		});

	m_pServer = nullptr;//不用加锁
	/*
	std::lock_guard lock(m_pServer->m_Sessions.m_setSessionMutex);
	m_pServer->m_Sessions.m_setSession.erase(this->m_pWsSession->m_pSession);
	LOG(INFO) << "删除Session，剩余" << m_pServer->m_Sessions.m_setSession.size();

	m_pServer->m_space.setEntity.erase(&m_entity);
	m_pServer = nullptr;
	*/
}
template void MySession::Send(const MsgLoginRet&);
template void MySession::Send(const MsgNotifyPos&);
template void MySession::Send(const MsgChangeSkeleAnim&);



template class WebSocketSession<MySession>;
template void WebSocketSession<MySession>::OnInit<MyServer>(Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >& refSession, MyServer& server);
template class WebSocketEndpoint<MySession, Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> > >;