#pragma once
#include "../IocpNetwork/WebSocketSession.h"
#include <deque>
class GateServer;
/// <summary>
/// 自己是服务器，浏览器H5游戏客户端通过该WebSocket协议连上来的一个连接
/// </summary>
class GateSession
{
public:
	using CompeletionKeySession = WebSocketSession<GateSession>;
	GateSession(CompeletionKeySession& ref) :m_refSession(ref)
	{

	}
	//int OnRecv(CompeletionKeySession&, const void* buf, int len);
	/// <summary>
	/// WebSocket收到一个完整二进制包
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	void OnRecvWsPack(const void* buf, const int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, GateServer&);
	template<class T> std::deque<T>& GetQueue();
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	CompeletionKeySession& m_refSession;
	//uint32_t m_snSend = 0;
};

