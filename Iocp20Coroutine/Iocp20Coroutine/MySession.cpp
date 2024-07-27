#include <glog/logging.h>

//#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "MySession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
//#include "MsgQueue.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"

//template<MySession>
//std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;
template Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >;
template class WebSocketSession<MySession>;
template void WebSocketSession<MySession>::OnInit<MyServer>(Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >& refSession, MyServer& server);
template class WebSocketEndpoint<MySession, Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> > >;

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



MySession::MySession() : m_msgQueue(this)
{
}
void MySession::OnRecvWsPack(const void *buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
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
	case MsgId::Move://
	{
		const auto msg = obj.as<MsgMove>();
		pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
	}
	break;
	case MsgId::Say:
	{
		const auto msg = obj.as<MsgSay>();
		pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
	}
	break;
	default:
		LOG(ERROR) << "没处理msgId:" << msgId;
		assert(false);
		break;
	}
}
void MySession::OnInit(WebSocketSession<MySession>& refWsSession, MyServer& server)
{
	server.m_Sessions.AddSession(refWsSession.m_pSession, [this, &refWsSession, &server]()
		{
			m_pServer = &server;
			m_pWsSession = &refWsSession;

			m_entity.Init(5, m_pServer->m_space, AiCo::Idle, "altman-blue");
			m_entity.AddComponent(this);
			m_pServer->m_space.setEntity.insert(&m_entity);			
		});
}

void MySession::OnDestroy()
{
	m_entity.OnDestroy();
	m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
		{
			m_pServer->m_space.setEntity.erase(&m_entity);
		});

	m_pServer = nullptr;//不用加锁
}
template void MySession::Send(const MsgLoginRet&);
template void MySession::Send(const MsgNotifyPos&);
template void MySession::Send(const MsgChangeSkeleAnim&);
template void MySession::Send(const MsgSay&);



