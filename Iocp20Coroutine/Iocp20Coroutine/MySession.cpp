#include <glog/logging.h>

#include "IocpNetwork/ServerTemplate.h"
#include "IocpNetwork/ListenSocketCompeletionKey.cpp"
#include "IocpNetwork/SessionSocketCompeletionKey.cpp"
#include "MySession.h"

//#include <iostream>
#include <cassert>

#include "websocketfiles-master/src/ws_endpoint.h"
#include <codecvt>
#include "MsgQueue.h"
#include "Space.h"
#include "CoTimer.h"

//template<MySession>
//std::set<Iocp::SessionSocketCompeletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;


template bool Iocp::Server::Init<WebSocketSession>();
//template class Iocp::ListenSocketCompeletionKey<MySession>;
//template Iocp::ListenSocketCompeletionKey::AcceptEx<MySession>(Iocp::Overlapped* pAcceptOverlapped, SOCKET socketListen);
template class Iocp::SessionSocketCompeletionKey<WebSocketSession>;

void net_write_cb(char* buf, int64_t size, void* wd)
{
	static_cast<Iocp::SessionSocketCompeletionKey<WebSocketSession>*>(wd)->Send(buf, size);
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
			LOG(INFO) << "WebSocketEndpoint - recv an unknown opcode.";
			break;
		}

		msgpack::object_handle oh = msgpack::unpack(frame_payload.bytes(), frame_payload.length());//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
		LOG(INFO) << obj;
		auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompeletionKey<WebSocketSession>*>(this->nt_work_data_);
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
void WebSocketSession::Send(const T& ref)
{
	m_webSocketEndpoint->Send(ref);
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

		Broadcast<MsgNotifyPos,WebSocketSession>(MsgNotifyPos(pEntity, x, z));
	}
}

WebSocketSession::WebSocketSession() : m_entity(5, g_space, TraceEnemy), m_msgQueue(this)
{
}

void WebSocketSession::OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession>& refSession)
{
	std::lock_guard lock(g_setSessionMutex<WebSocketSession>);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint(net_write_cb, &refSession));

	m_pSession = &refSession;
	g_space.setEntity.insert(&m_entity);
}
int WebSocketSession::OnRecv(Iocp::SessionSocketCompeletionKey<WebSocketSession>& refSession, const char buf[], int len)
{
	m_webSocketEndpoint->from_wire(buf, len);
	return len;
}

void WebSocketSession::OnDestroy()
{
	std::lock_guard lock(g_setSessionMutex<WebSocketSession>);
	g_setSession<WebSocketSession>.erase(this->m_pSession);
	g_space.setEntity.erase(&m_entity);
	LOG(INFO) << "删除Session，剩余" << g_setSession<WebSocketSession>.size();

}

template void WebSocketSession::Send(const MsgLoginRet&);
template void WebSocketSession::Send(const MsgNotifyPos&);

template<class T,class T_Session>
void Broadcast(const T& msg)
{
	std::lock_guard lock(g_setSessionMutex<T_Session>);
	for (auto p : g_setSession<T_Session>)
	{
		p->Session.Send(msg);
	}
}

template void Broadcast<MsgLoginRet,WebSocketSession>(const MsgLoginRet&);
template void Broadcast<MsgNotifyPos, WebSocketSession>(const MsgNotifyPos&);
template void Broadcast<MsgChangeSkeleAnim, WebSocketSession>(const MsgChangeSkeleAnim&);