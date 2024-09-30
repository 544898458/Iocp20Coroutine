#pragma once
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include <set>
class WorldSession;

/// <summary>
/// ���GameSvr��Ķ��󣬶�Ӧһ��GateSvr����Ϸ�ͻ��˵�����
/// </summary>
class PlayerGateSession_World
{
public:
	PlayerGateSession_World(WorldSession& ref, const uint64_t idPlayerGateSession) :m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession)
	{

	}
	PlayerGateSession_World(const PlayerGateSession_World&) = delete;
	PlayerGateSession_World(PlayerGateSession_World&& ref)noexcept :m_refSession(ref.m_refSession), m_idPlayerGateSession(ref.m_idPlayerGateSession)
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
/// ���߼��̣߳�����̨�����̣߳�����
/// </summary>
/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	CoTask<int> CoLogin(const MsgLogin msg);
	
	CoTask<int> m_coLogin;

	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	WorldSession& m_refSession;
	const uint64_t m_idPlayerGateSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
	uint32_t m_snSend = 0;
};

