#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"

enum MsgId;
//class Client_GateToWorld;

class ClientSession_GameToWorld
{
public:
	using Session = Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>;
	ClientSession_GameToWorld(Session&) {}
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
	int OnRecv(Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld>& refSession, const void* buf, int len);
	void OnRecvPack(const void* buf, int len);
	void OnDestroy()
	{

	}
	
	//Client_GateToWorld* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<ClientSession_GameToWorld> *m_pSession = nullptr;
	
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	void OnAppExit();

	template<class T> std::deque<T>& GetQueue();
	uint32_t m_snRecv = 0;

private:
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgChangeMoneyResponce& msg);

	
	/// <summary>
	/// 这里保存的都是解析后的消息明文,反序列化在网络线程，处理明文消息在逻辑线程
	/// </summary>
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgChangeMoneyResponce> m_queueConsumeMoneyResponce;
	MsgQueueMsgPack<ClientSession_GameToWorld> m_MsgQueue;
};