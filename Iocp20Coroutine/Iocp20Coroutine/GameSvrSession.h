#pragma once
#include "MyMsgQueue.h"
#include "SpEntity.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../IocpNetwork/SessionSocketCompletionKey.h"

class GameSvr;
class GameSvrSession
{
public:
	//using WebSocketGameSession = WebSocketSession<GameSvrSession>;
	using WebSocketGameSession = Iocp::SessionSocketCompletionKey<GameSvrSession>;
	GameSvrSession(WebSocketGameSession& refWsSession) {}

	/// <summary>
	/// WebSocket收到一个完整二进制包
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	int OnRecv(WebSocketGameSession&,const void* buf, const int len);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="refWsSession"></param>
	/// <param name="server"></param>
	void OnInit(WebSocketGameSession& refWsSession, GameSvr& server);
	void OnDestroy();
	void Erase(SpEntity spEntity);
	template<class T>
	void Send(const T& ref);
	/// <summary>
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	std::set<SpEntity> m_vecSpEntity;

	GameSvr* m_pServer = nullptr;
	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;

	template<class T>
	std::deque<T>& GetQueue();

	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;
private:
	void OnRecvPack(const void* buf, const int len);
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const MsgGate转发& msg);
	template<class T_Msg> void RecvMsg(msgpack::object& obj);


	CoTask<int> CoAddRole();
	CoTask<int> CoAddBuilding();
	/// <summary>
	/// 这里保存的都是解析后的消息明文
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgSelectRoles> m_queueSelectRoles;
	std::deque<MsgAddRole> m_queueAddRole;
	std::deque<MsgAddBuilding> m_queueAddBuilding;
	std::deque<MsgGate转发> m_queueGate转发;

	MsgQueueMsgPack<GameSvrSession> m_MsgQueue;
private:
	WebSocketGameSession* m_pWsSession = nullptr;
};
