#include <glog/logging.h>

#include "MySession.h"
#include "IocpNetwork/Server.cpp"
#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "IocpNetwork/SessionSocketCompeletionKey.cpp"

//#include <iostream>
#include <cassert>

#include "websocketfiles-master/src/ws_endpoint.h"
#include <codecvt>
#include "MsgQueue.h"
#include "Space.h"

std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
std::mutex g_setSessionMutex;

template class Iocp::Server<MySession>;
template class Iocp::ListenSocketCompeletionKey<MySession>;
template class Iocp::SessionSocketCompeletionKey<MySession>;

void net_write_cb(char* buf, int64_t size, void* wd)
{
	static_cast<Iocp::SessionSocketCompeletionKey<MySession>*>(wd)->Send(buf, size);
}
/// <summary>
/// https://github.com/basson099/websocketfiles
/// https://blog.csdn.net/qq_39540028/article/details/104493049
/// 此开源库才几百行，就能把原有在C++在TCP服务器快速改成WebSocket服务器
/// </summary>
class MyWebSocketEndpoint :public WebSocketEndpoint
{
public:
	MyWebSocketEndpoint(nt_write_cb write_cb, void* work_data) :WebSocketEndpoint(write_cb, work_data)
	{
	}
	
	virtual int32_t user_defined_process(WebSocketPacket& packet, ByteBuffer& frame_payload)override
	{
		switch (packet.get_opcode())
		{
		case WebSocketPacket::WSOpcode_Continue:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Continue opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Text:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Text opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Binary:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Binary opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Close:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Close opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			return 0;
			break;
		case WebSocketPacket::WSOpcode_Ping:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Ping opcode." << std::endl;
			user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Pong:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Pong opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		default:
			LOG(INFO) << "WebSocketEndpoint - recv an unknown opcode." ;
			break;
		}

		msgpack::object_handle oh = msgpack::unpack(frame_payload.bytes() , frame_payload.length());//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
		LOG(INFO) << obj ;
		auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompeletionKey<MySession>*>(this->nt_work_data_);
		switch (msgId)
		{
		case MsgId::Login:
		{
			const auto msg = obj.as<MsgLogin>();
			pSessionSocketCompeletionKey->Session.m_msgQueue.Push(msg);
		}
		break;
		case MsgId::Move:
		{
			const auto msg = obj.as<MsgMove>();
			pSessionSocketCompeletionKey->Session.m_msgQueue.Push(msg);
		}
		break;
		}

		return 0;
	}
	template<class T>
	void Send(const T& ref)
	{
		WebSocketPacket wspacket;
		// set FIN and opcode
		wspacket.set_fin(1);
		wspacket.set_opcode(0x02);// packet.get_opcode());
		// set payload data

		std::stringstream buffer;
		msgpack::pack(buffer, ref);
		buffer.seekg(0);

		// deserialize the buffer into msgpack::object instance.
		std::string str(buffer.str());
		wspacket.set_payload(str.data(), str.size());
		ByteBuffer output;
		// pack a websocket data frame
		wspacket.pack_dataframe(output);
		// send to client
		to_wire(output.bytes(), output.length());
	}
};

template<class T>
void MySession::Send(const T& ref)
{
	m_webSocketEndpoint->Send(ref);

}

CoTask<int> TraceEnemy(Entity* pEntity, float& x, float& z,bool &stop)
{
	while (true)
	{
		co_yield 0;
		if (stop) 
		{

			LOG(INFO) << "TraceEnemy协程正常退出" ;
			co_return 0;
		}
		x -= 0.01;

		MsgNotifyPos msg = { (int)NotifyPos ,(uint64_t)pEntity, x,z };
		Broadcast(msg);
	}
}

MySession::MySession() : m_entity(5, g_space, TraceEnemy), m_msgQueue(this)
{
}

void MySession::OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession)
{
	std::lock_guard lock(g_setSessionMutex);
	m_webSocketEndpoint.reset( new MyWebSocketEndpoint(net_write_cb, &refSession));
	g_setSession.insert(&refSession);
	//#include <glog/logging.h>
	LOG(INFO) << "添加Session，剩余" << g_setSession.size();
	m_pSession = &refSession;
	g_space.mapEntity[(long)this] = &m_entity;
}
int MySession::OnRecv(Iocp::SessionSocketCompeletionKey<MySession>& refSession, const char buf[], int len)
{
	m_webSocketEndpoint->from_wire(buf, len);
	return len;
}

void MySession::OnDestroy()
{
	std::lock_guard lock(g_setSessionMutex);
	g_setSession.erase(this->m_pSession);
	g_space.mapEntity.erase((long)this);
	LOG(INFO) << "删除Session，剩余" << g_setSession.size();
	
}

template void MySession::Send(const MsgLoginRet&);
template void MySession::Send(const MsgNotifyPos&);

template<class T>
void Broadcast(const T& msg)
{
	std::lock_guard lock(g_setSessionMutex);
	for (auto p : g_setSession)
	{
		p->Session.Send(msg);
	}
}

template void Broadcast(const MsgLoginRet&);
template void Broadcast(const MsgNotifyPos&);