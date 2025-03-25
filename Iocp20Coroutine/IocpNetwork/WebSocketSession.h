#pragma once
#include <glog/logging.h>
#include "../websocketfiles-master/src/ws_endpoint.h"
#include "SessionSocketCompletionKey.h"
#include <deque>
#include "SslTlsSvr.h"
#include "MsgPack.h"
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
class MyWebSocketEndpoint :public WebSocketEndpoint<T_Callback, T_Data>
{
public:
	MyWebSocketEndpoint(T_Callback* write_cb, T_Data* work_data) :WebSocketEndpoint< T_Callback, T_Data>(write_cb, work_data)
	{
	}

	virtual int32_t user_defined_process(WebSocketPacket& packet, ByteBuffer& frame_payload)override
	{
		switch (packet.get_opcode())
		{
		//case WebSocketPacket::WSOpcode_Continue:
		//	// add your process code here
		//	//std::cout << "WebSocketEndpoint - recv a Continue opcode." << std::endl;
		//	//user_defined_process(packet, frame_payload);
		//	break;
		//case WebSocketPacket::WSOpcode_Text:
		//	// add your process code here
		//	//std::cout << "WebSocketEndpoint - recv a Text opcode." << std::endl;
		//	//user_defined_process(packet, frame_payload);
		//	break;
		case WebSocketPacket::WSOpcode_Binary:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Binary opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			this->nt_write_cb_->OnRecvWsPack(frame_payload.bytes(), frame_payload.length());
			break;
		//case WebSocketPacket::WSOpcode_Close:
		//	// add your process code here
		//	//std::cout << "WebSocketEndpoint - recv a Close opcode." << std::endl;
		//	//user_defined_process(packet, frame_payload);
		//	return 0;
		//	break;
		//case WebSocketPacket::WSOpcode_Ping:
		//	// add your process code here
		//	//std::cout << "WebSocketEndpoint - recv a Ping opcode." << std::endl;
		//	//user_defined_process(packet, frame_payload);
		//	break;
		//case WebSocketPacket::WSOpcode_Pong:
		//	// add your process code here
		//	//std::cout << "WebSocketEndpoint - recv a Pong opcode." << std::endl;
		//	//user_defined_process(packet, frame_payload);
		//	break;
		default:
			LOG(INFO) << "WebSocketEndpoint - recv an unknown opcode.";
			WebSocketEndpoint<T_Callback, T_Data>::user_defined_process(packet, frame_payload);
			break;
		}

		//this->nt_write_cb_->OnRecvWsPack(frame_payload.bytes(), frame_payload.length());

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

		WebSocketPacket wspacket;
		// set FIN and opcode
		wspacket.set_fin(1);
		wspacket.set_opcode(0x02);// packet.get_opcode());
		//set payload data
		wspacket.set_payload((char*)buf, len);
		ByteBuffer output;
		// pack a websocket data frame
		wspacket.pack_dataframe(output);
		//send to client
		this->to_wire(output.bytes(), output.length());
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
			this->to_wire(buf, (int)len);
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
	WebSocketSession(Session&ref) :m_Session(*this), m_refSession(ref){}
	/// <summary>
	/// ���캯����OnInit������ŵ��ã���ʼ�������������Ҹ��ط�д��ֻҪ�ܱ���ͨ����Ч������ȫ��ͬ
	/// </summary>
	//WebSocketSession();
	virtual ~WebSocketSession() {}
	template<class T_Server>
		requires requires(WebSocketSession<T_Session>& refWsSesson, T_Session& refSession, T_Server& server)
	{
		//refSession.OnRecvWsPack((const char[])0, (const int )0	);
		refSession.OnInit(server);
	}
	void OnInit(T_Server&);
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

	/// <summary>
	/// ����WebSocket����
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	void Send(const void* buf, const int len)
	{
		m_webSocketEndpoint->Send(buf, len);
		//m_refSession.Send(buf, len);//
	}
	void SendWebSocket����������(const void* buf, const int len)
	{
		if (m_bSslTls)
		{
			{
				const auto i32�Ѵ������� = m_SslTls.��Ҫ����ǰ�˵����Ľ���Ssl����(buf, len);
				if (i32�Ѵ������� != len)
				{
					LOG(ERROR) << "i32�Ѵ�������=" << i32�Ѵ������� << ",len=" << len << ",m_SslTls.����OK=" << m_SslTls.����OK();
					_ASSERT(false);
					m_refSession.CloseSocket();
				}
			}
			��ȡ׼������ǰ�˵����Ĳ�����ǰ��();
		}
		else
		{
			m_refSession.Send(buf, len);
		}

	}
	void ��ȡ׼������ǰ�˵����Ĳ�����ǰ��()
	{

		char buf����[4096];
		int len���� = m_SslTls.��ȡ׼������ǰ�˵�����(buf����);
		if (0 < len����)
		{
			LOG(INFO) << "�����ĸ�ǰ��,len=" << len����;
			m_refSession.Send(buf����, len����);
		}
	}
	template<class T>
	void Send(const T& ref)
	{
		ref.msg.sn = ++m_Session.m_snSendToClient;
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len)
			{
				this->Send((const char*)buf, len); 
			}, false);
	}
	bool Process() { return m_Session.Process(); }

	/// <summary>
	/// ��ԴWebSocket��
	/// </summary>
	std::unique_ptr<MyWebSocketEndpoint<T_Session, WebSocketSession<T_Session>>> m_webSocketEndpoint;
	Session& m_refSession;
	T_Session m_Session;
	
	SslTlsSvr m_SslTls;
private:
	const bool m_bSslTls = true;//wss����ws

};