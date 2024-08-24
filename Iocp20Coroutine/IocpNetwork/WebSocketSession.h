#pragma once
#include <glog/logging.h>
#include "../websocketfiles-master/src/ws_endpoint.h"
#include "SessionSocketCompletionKey.h"
#include <deque>

//void net_write_cb(char* buf, int64_t size, void* wd)
//{
//	static_cast<Iocp::
// <WebSocketSession<MySession> >*>(wd)->Send(buf, size);
//}
/// <summary>
/// https://github.com/basson099/websocketfiles
/// https://blog.csdn.net/qq_39540028/article/details/104493049
/// �˿�Դ��ż����У����ܰ�ԭ����C++��TCP���������ٸĳ�WebSocket������
/// </summary>

template<class T_Callback, class T_Data>
	requires requires(T_Callback& cb)
{
	cb.OnRecvWsPack(nullptr, 0);
}
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

		this->nt_write_cb_->OnRecvWsPack(frame_payload.bytes(), frame_payload.length());

		return 0;
	}

	/// <summary>
	/// ����һ��WebSocket���ݰ����ͻ���
	/// </summary>
	/// <param name="ref"></param>
	void Send(const void* buf, const int len)
	{
		if (0 >= len)
		{
			LOG(WARNING) << "len = 0";
			return;
		}
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
		if (!this->ws_handshake_completed_)
		{
			std::lock_guard lock(m_mutexQueueSendBuf);
			std::vector<char> sendBuf;
			sendBuf.resize(len);//���ܻ��ڴ�
			const char* begin = (char*)buf;
			std::copy(begin, begin + len, &sendBuf[0]);//memcpy�滻
			m_queueSendBuf.push_back(sendBuf);

			LOG(INFO) << "WebSocket ��û����,����len=" << len << ",m_queueSendBuf.size=" << m_queueSendBuf.size();
			return;
		}
		this->to_wire(buf, len);
	}
	virtual void onHandShakeCompleted()override
	{
		std::lock_guard lock(m_mutexQueueSendBuf);
		while (!m_queueSendBuf.empty())
		{
			auto& sendBuf = m_queueSendBuf.front();
			const auto buf = &sendBuf[0];
			const auto len = sendBuf.size();
			LOG(INFO) << "WebSocket ���ֳɹ�,�������len=" << len << ",m_queueSendBuf.size=" << m_queueSendBuf.size();
			this->to_wire(buf, len);
			m_queueSendBuf.pop_front();
		}
	}
	std::mutex m_mutexQueueSendBuf;
	std::deque<std::vector<char>> m_queueSendBuf;
};

/// <summary>
/// ��Ӧһ����������
/// </summary>
template<class T_Session>
//	requires requires(T_Session& refSession)
//{
//	refSession.Process();
//}
class WebSocketSession
{
public:
	using Session = Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>;
	WebSocketSession(Session&):m_Session(*this){}
	/// <summary>
	/// ���캯����OnInit������ŵ��ã���ʼ�������������Ҹ��ط�д��ֻҪ�ܱ���ͨ����Ч������ȫ��ͬ
	/// </summary>
	//WebSocketSession();
	virtual ~WebSocketSession() {}
	template<class T_Server>
		requires requires(WebSocketSession<T_Session>& refWsSesson, T_Session& refSession, T_Server& server)
	{
		//refSession.OnRecvWsPack((const char[])0, (const int )0	);
		refSession.OnInit(refWsSesson, server);
	}
	void OnInit(Session& refSession, T_Server&);
	/// <summary>
	/// �û��Զ��庯���������Ǵ����ݣ���������û�У������WebSocketЭ�鸺��Ĺ���
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	int OnRecv(Session& refSession, const void* buf, int len);
	/// <summary>
/// ��ȫ������set��ɾ�����ӣ���ȫ��Space��ɾ��ʵ��
/// </summary>
	void OnDestroy();
	void Send(const void* buf, const int len)
	{
		m_webSocketEndpoint->Send(buf, len);
	}
	template<class T>
	void Send(const T& ref)
	{
		m_Session.Send(ref);
	}
	void Process() { m_Session.Process(); }

	/// <summary>
	/// ��ԴWebSocket��
	/// </summary>
	std::unique_ptr<MyWebSocketEndpoint<T_Session, Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>>> m_webSocketEndpoint;
	T_Session m_Session;
	Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>* m_pSession;
private:


};