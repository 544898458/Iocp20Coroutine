#pragma once
#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include <msgpack.hpp>
#include "MyMsgQueue.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"

enum MsgId;
class WorldClient;

class WorldClientSession
{
public:
	void OnInit(Iocp::SessionSocketCompletionKey<WorldClientSession>& session, WorldClient& refWorldClient)
	{
		m_pWorldClient = &refWorldClient;
		m_pSession = &session;
		//session.Session.m_MsgQueue.m_pWorldClient = &refWorldClient;
	}

	/// <summary>
	/// 网络线程（多线程）调用
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>
	int OnRecv(Iocp::SessionSocketCompletionKey<WorldClientSession>& refSession, const void* buf, int len)
	{
		const void* bufPack(nullptr);
		int lenPack(0);
		std::tie(bufPack, lenPack) = Iocp::OnRecv2(buf, len);
		if (lenPack > 0 && nullptr != bufPack)
		{
			OnRecvPack(bufPack, lenPack);
		}

		return lenPack;
	}
	void OnRecvPack(const void* buf, int len);
	void OnDestroy()
	{

	}
	
	WorldClient* m_pWorldClient = nullptr;
	Iocp::SessionSocketCompletionKey<WorldClientSession> *m_pSession = nullptr;
	
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	template<class T> std::deque<T>& GetQueue();
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
	MsgQueueMsgPack<WorldClientSession> m_MsgQueue;
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompletionKey<WorldClientSession>& session)
	{

	}
	static std::function<void(MsgSay const&)> m_funBroadcast;
};
