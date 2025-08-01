#pragma once
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include <set>
class WorldSessionFromGate;

/// <summary>
/// 这个GameSvr里的对象，对应一个GateSvr对游戏客户端的连接
/// </summary>
class PlayerGateSession_World
{
public:
	PlayerGateSession_World(WorldSessionFromGate& ref, const uint64_t idPlayerGateSession) :m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_funCancle("PlayerGateSession_World")
	{

	}
	PlayerGateSession_World(const PlayerGateSession_World&) = delete;
	PlayerGateSession_World(PlayerGateSession_World&& ref)noexcept :m_refSession(ref.m_refSession), m_idPlayerGateSession(ref.m_idPlayerGateSession), m_funCancle("PlayerGateSession_World")
	{

	}
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	const std::string& NickName()const { return m_nickName; }

	template<class T> void SendToGate转发(const T& refMsg);
	template<class T> void SendToGame转发(const T& refMsg);
	uint8_t m_snRecv = 0;

private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
/// 主逻辑线程（控制台界面线程）调用
/// </summary>
/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgGateDeleteSessionResponce& msg);
	CoTask<int> CoLogin(const MsgLogin msg);
	
	CoTask<int> m_coLogin;
	FunCancel安全 m_funCancle;
	bool m_bLoginOk = false;

	WorldSessionFromGate& m_refSession;
	const uint64_t m_idPlayerGateSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
	uint8_t m_snSend = 0;
};

