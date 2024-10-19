#pragma once
#include "../Iocp20Coroutine/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "SpEntity.h"
#include "Space.h"
#include <set>
class GameSvrSession;


/// <summary>
/// ���GameSvr��Ķ��󣬶�Ӧһ��GateSvr����Ϸ�ͻ��˵�����
/// </summary>
class PlayerGateSession_Game
{
public:
	PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession);
	PlayerGateSession_Game(const PlayerGateSession_Game&) = delete;
	PlayerGateSession_Game(PlayerGateSession_Game&& ref) = delete;
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	void Erase(SpEntity& spEntity);
	template<class T> void Send(const T& ref);
	void Say(const std::string& str);
	const std::string& NickName()const { return m_strNickName; }
	void EnterSpace(Space& refSpace, const std::string& strNickName);
	void SendMy��Դ();

	uint32_t m_snRecv = 0;
	const uint64_t m_idPlayerGateSession;
	Space &m_Space���˾���;
	/// <summary>
	/// ��ǰ���������ĸ�Space��
	/// </summary>
	Space* m_pCurSpace = nullptr;
	/// <summary>
	/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
	/// </summary>
	std::set<SpEntity> m_vecSpEntity;

	int m_iȼ���� = 0;

private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
/// ���߼��̣߳�����̨�����̣߳�����
/// </summary>
/// <param name="msg"></param>
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const Msg�ɼ�& msg);

	void ForEachSelected(std::function<void(Entity& ref)> fun);
	//CoTask<int> CoAddRole();
	CoTask<int> CoAddBuilding(const ������λ���� ����);

	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	GameSvrSession& m_refSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_strNickName;
	uint32_t m_snSend = 0;
	FunCancel m_funCancel���˾���;
};

