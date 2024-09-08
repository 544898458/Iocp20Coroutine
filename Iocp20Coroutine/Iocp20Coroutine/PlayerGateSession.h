#pragma once
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "SpEntity.h"
#include <set>
class GameSvrSession;
class Space;

/// <summary>
/// 这个GameSvr里的对象，对应一个GateSvr对游戏客户端的连接
/// </summary>
class PlayerGateSession
{
public:
	PlayerGateSession(GameSvrSession& ref, const uint64_t idPlayerGateSession) :m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession)
	{

	}
	PlayerGateSession(const PlayerGateSession&) = delete;
	PlayerGateSession(PlayerGateSession&& ref)noexcept :m_refSession(ref.m_refSession), m_idPlayerGateSession(ref.m_idPlayerGateSession)
	{

	}
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy(Space& refSpace);
	void Erase(SpEntity& spEntity);
	template<class T> void Send(const T& ref);
	const std::string& NickName()const { return m_nickName; }

	uint32_t m_snRecv = 0;

private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
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
	//template<class T> std::deque<T>& GetQueue();
	CoTask<int> CoAddRole();
	CoTask<int> CoAddBuilding();


	//MsgQueueMsgPack<PlayerGateSession> m_MsgQueue;
	//std::deque<MsgLogin> m_queueLogin;
	//std::deque<MsgMove> m_queueMove;
	//std::deque<MsgSay> m_queueSay;
	//std::deque<MsgSelectRoles> m_queueSelectRoles;
	//std::deque<MsgAddRole> m_queueAddRole;
	//std::deque<MsgAddBuilding> m_queueAddBuilding;

	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	/// <summary>
/// 加入Space空间的实体（玩家角色）
/// </summary>
	std::set<SpEntity> m_vecSpEntity;
	GameSvrSession& m_refSession;
	const uint64_t m_idPlayerGateSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
	uint32_t m_snSend = 0;
};

