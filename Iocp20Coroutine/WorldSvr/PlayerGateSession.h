#pragma once
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include <set>
class WorldSession;

/// <summary>
/// 这个GameSvr里的对象，对应一个GateSvr对游戏客户端的连接
/// </summary>
class PlayerGateSession
{
public:
	PlayerGateSession(WorldSession& ref, const uint64_t idPlayerGateSession) :m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession)
	{

	}
	PlayerGateSession(const PlayerGateSession&) = delete;
	PlayerGateSession(PlayerGateSession&& ref)noexcept :m_refSession(ref.m_refSession), m_idPlayerGateSession(ref.m_idPlayerGateSession)
	{

	}
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
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
	
	CoTask<int> CoAddRole();
	CoTask<int> CoAddBuilding();

	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	WorldSession& m_refSession;
	const uint64_t m_idPlayerGateSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
	uint32_t m_snSend = 0;
};

