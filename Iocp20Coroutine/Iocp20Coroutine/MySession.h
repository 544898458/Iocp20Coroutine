#pragma once
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "../IocpNetwork/WebSocketSession.h"

class MyServer;
class MySession
{
public:
	//MySession();
	void OnRecvWsPack(const void* buf, const int len);
	void OnInit(WebSocketSession<MySession>& refWsSession, MyServer& server);
	void OnDestroy();
	void Erase(SpEntity spEntity);
	template<class T>
	void Send(const T& ref);
	/// <summary>
	/// 加入Space空间的实体（玩家角色）
	/// </summary>
	std::set<SpEntity> m_vecSpEntity;
	
	MyServer* m_pServer = nullptr;
	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;

	/// <summary>
	/// 工作线程中（单线程）调用
	/// </summary>
	void Process();
	CoTask<int> m_coRpc;
private:
	/// <summary>
	/// 主逻辑线程（控制台界面线程）调用
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);

	CoTask<int> CoAddRole();
	template<class T>
	std::deque<T>& GetQueue();
	/// <summary>
	/// 这里保存的都是解析后的消息明文
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgSelectRoles> m_queueSelectRoles;
	std::deque<MsgAddRole> m_queueAddRole;

	MsgQueue m_MsgQueue;
private:
	template<class T>
	void PushMsg(const msgpack::object& obj);
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};
