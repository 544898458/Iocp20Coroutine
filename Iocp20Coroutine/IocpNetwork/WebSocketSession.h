#pragma once
#include <glog/logging.h>
#include "../websocketfiles-master/src/ws_endpoint.h"
#include "SessionSocketCompeletionKey.h"


//void net_write_cb(char* buf, int64_t size, void* wd)
//{
//	static_cast<Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession> >*>(wd)->Send(buf, size);
//}
/// <summary>
/// https://github.com/basson099/websocketfiles
/// https://blog.csdn.net/qq_39540028/article/details/104493049
/// �˿�Դ��ż����У����ܰ�ԭ����C++��TCP���������ٸĳ�WebSocket������
/// </summary>
template<class T_Callback, class T_Data>
class MyWebSocketEndpoint :public WebSocketEndpoint< T_Callback, T_Data>
{
public:
	MyWebSocketEndpoint(T_Callback* write_cb, T_Data* work_data) :WebSocketEndpoint< T_Callback, T_Data>(write_cb, work_data)
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

		//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompeletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
		//pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
		this->nt_write_cb_->OnRecvWsPack(frame_payload.bytes(), frame_payload.length());
		/*
		msgpack::object_handle oh = msgpack::unpack(frame_payload.bytes(), frame_payload.length());//û�ж�Խ�磬Ҫ��try
		msgpack::object obj = oh.get();
		const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
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
		*/
		return 0;
	}
	/// <summary>
	/// ����һ��WebSocket���ݰ����ͻ���
	/// </summary>
	/// <param name="ref"></param>
	void Send(const void *buf, const int len)
	{
		//WebSocketPacket wspacket;
		//// set FIN and opcode
		//wspacket.set_fin(1);
		//wspacket.set_opcode(0x02);// packet.get_opcode());
		// set payload data

		//std::stringstream buffer;
		//msgpack::pack(buffer, ref);
		//buffer.seekg(0);

		//// deserialize the buffer into msgpack::object instance.
		//std::string str(buffer.str());
		//wspacket.set_payload(str.data(), str.size());
		//ByteBuffer output;
		//// pack a websocket data frame
		//wspacket.pack_dataframe(output);
		// send to client
		//this->to_wire(output.bytes(), output.length());
		this->to_wire(buf, len);
	}
};

/// <summary>
/// ��Ӧһ����������
/// </summary>
template<class T_Session>
//requires requires(T_Session& refSession)
//{
//	refSession.OnRecvWsPack((const char[])0, (const int )0	);
//}
class WebSocketSession
{
public:
	/// <summary>
	/// ���캯����OnInit������ŵ��ã���ʼ�������������Ҹ��ط�д��ֻҪ�ܱ���ͨ����Ч������ȫ��ͬ
	/// </summary>
	//WebSocketSession();
	virtual ~WebSocketSession() {}
	template<class T_Server>
	void OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, T_Server&);
	/// <summary>
	/// �û��Զ��庯���������Ǵ����ݣ���������û�У������WebSocketЭ�鸺��Ĺ���
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	int OnRecv(Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>& refSession, const void* buf, int len);
	/// <summary>
/// ��ȫ������set��ɾ�����ӣ���ȫ��Space��ɾ��ʵ��
/// </summary>
	void OnDestroy();
	void Send(const void *buf, const int len)
	{
		m_webSocketEndpoint->Send(buf, len);
	}
	template<class T>
	void Send(const T& ref)
	{
		m_Session.Send(ref);
	}
	/// <summary>
	/// ��ԴWebSocket��
	/// </summary>
	std::unique_ptr<MyWebSocketEndpoint<T_Session, Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>>> m_webSocketEndpoint;
	T_Session m_Session;
	Iocp::SessionSocketCompeletionKey<WebSocketSession<T_Session>>* m_pSession;
private:


};