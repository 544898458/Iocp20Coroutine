#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/MsgPack.h"

enum MsgId;
//class Client_GateToWorld;

class ClientSession_GateToWorld
{
public:
	using Session = Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>;
	ClientSession_GateToWorld(Session& ref) :m_refSession(ref) {}
	//void OnInit(Session& session, WorldClient& refWorldClient)
	//{
	//	m_pWorldClient = &refWorldClient;
	//	m_pSession = &session;
	//}

	/// <summary>
	/// 网络线程（多线程）调用
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>
	int OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& refSession, const void* buf, int len);
	void OnDestroy()
	{
	}

	//Client_GateToWorld* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>* m_pSession = nullptr;

	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	template<class T> std::deque<T>& GetQueue();
	uint32_t m_snRecv = 0;
	template<class T>
	void Send(const T& ref)
	{
		++m_snSend;
		ref.msg.sn = (m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
	}
private:
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgGateDeleteSession& msg);
	void OnRecv(const MsgGate转发& msg);
	void OnRecv(const Msg在线人数& msg);

	void OnRecvPack(const void* buf, int len);


	/// <summary>
	/// 这里保存的都是解析后的消息明文,反序列化在网络线程，处理明文消息在逻辑线程
	/// </summary>
	std::deque<MsgGateDeleteSession> m_queueGateDeleteSession;
	std::deque<MsgGate转发> m_queueGate转发;
	std::deque<Msg在线人数> m_queue在线人数;
	MsgQueueMsgPack<ClientSession_GateToWorld> m_MsgQueue;
	Session& m_refSession;
	uint32_t m_snSend = 0;
};
//class Client_GateToWorld
//{
//public:
//	void OnAdd(Iocp::SessionSocketCompletionKey<ClientSession_GateToWorld>& session)
//	{
//
//	}
//};
