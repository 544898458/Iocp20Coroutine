#pragma once
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include <set>
class WorldSessionFromGate;

/// <summary>
/// ���GameSvr��Ķ��󣬶�Ӧһ��GateSvr����Ϸ�ͻ��˵�����
/// </summary>
class PlayerGateSession_World
{
public:
	PlayerGateSession_World(WorldSessionFromGate& ref, const uint64_t idPlayerGateSession) :m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession)
	{

	}
	PlayerGateSession_World(const PlayerGateSession_World&) = delete;
	PlayerGateSession_World(PlayerGateSession_World&& ref)noexcept :m_refSession(ref.m_refSession), m_idPlayerGateSession(ref.m_idPlayerGateSession)
	{

	}
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	const std::string& NickName()const { return m_nickName; }

	template<class T> void SendToGateת��(const T& refMsg);
	template<class T> void SendToGameת��(const T& refMsg);
	uint32_t m_snRecv = 0;

private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
/// ���߼��̣߳�����̨�����̣߳�����
/// </summary>
/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgGateDeleteSessionResponce& msg);
	CoTask<int> CoLogin(const MsgLogin msg);
	
	CoTask<int> m_coLogin;

	//std::vector<CoTask<int>>	m_vecCoRpc;
	//std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	WorldSessionFromGate& m_refSession;
	const uint64_t m_idPlayerGateSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
	uint32_t m_snSend = 0;
};

