#pragma once
#include "../IocpNetwork/WebSocketSession.h"
#include <deque>
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
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
	uint64_t GetId()const { return (uint64_t)this; }
	//int OnRecv(CompeletionKeySession&, const void* buf, int len);
	/// <summary>
	/// WebSocket收到一个完整二进制包
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	void OnRecvWsPack(const void* buf, const int len);

	void OnDestroy();
	void OnInit(GateServer&);
	void OnRecvWorldSvr(const MsgLoginResponce& msg);
	void OnRecvWorldSvr(const MsgGateDeleteSession& msg);
	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	bool Process();
	template<class T> std::deque<T>& GetQueue();
	CompeletionKeySession& m_refSession;
	/// <summary>
	/// 这里保存的都是解析后的消息明文,反序列化在网络线程，处理明文消息在逻辑线程
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgGate转发> m_queueMsgGate转发;
	MsgQueueMsgPack<GateSession> m_MsgQueue;
	uint32_t m_snRecv = 0;
	uint32_t m_snSendToGameSvr = 0;
	uint32_t m_snSendToWorldSvr = 0;
	uint32_t m_snSendToClient = 0;//发给游戏前端
	bool m_bLoginOk = false;
private:
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgGate转发& msg);
	CoTask<int> CoLogin(MsgLogin msg, FunCancel& funCancel);

	CoTask<int> m_coLogin;
	FunCancel m_funCancelLogin;
	bool m_bClosed = false;
};

