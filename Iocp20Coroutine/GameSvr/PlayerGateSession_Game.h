#pragma once
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "SpEntity.h"
#include "Space.h"
#include <set>
class GameSvrSession;


/// <summary>
/// ���GameSvr��Ķ��󣬶�Ӧһ��GateSvr����Ϸ�ͻ��˵�����
/// </summary>
class PlayerGateSession_Game : public std::enable_shared_from_this<PlayerGateSession_Game>//���빫�м̳У�������Ч
{
public:
	PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession, const std::string& strNickName);
	PlayerGateSession_Game(const PlayerGateSession_Game&) = delete;
	PlayerGateSession_Game(PlayerGateSession_Game&& ref) = delete;
	void RecvMsg(const MsgId idMsg, const msgpack::object& obj);
	void Process();
	void OnDestroy();
	void �뿪Space(const bool b������);

	template<class T> void Send(const T& ref);
	void Say(const std::string& str, const SayChannel channel);
	void Sayϵͳ(const std::string& str);
	static void Sayϵͳ(const std::string& refStrNickName, const std::string& str);
	static void Say������ʾ(const std::string& refStrNickName, const std::string& str);
	//void Say������ʾ(const std::string& str);

	const std::string& NickName()const { return m_strNickName; }
	WpEntity EnterSpace(WpSpace wpSpace);
	void Send��Դ();
	uint16_t ���λ����() const;
	uint16_t ���λ������������е�() const;

	void ��������Buzz(const std::string& str�ı�);
	void ��������(const std::string& refStr����, const std::string& str�ı�);
	static void ��������(const std::string& refStrNickName, const std::string& refStr����, const std::string& str�ı�);

	void Send�����ӿ�(const Entity& refEntity);
	void ɾ��ѡ��(const uint64_t id);

	void ����Ի�(
		const std::string& strͷ����, const std::string& str������,
		const std::string& strͷ����, const std::string& str������,
		const std::string& str����, const bool b��ʾ�˳�������ť);
	void ����Ի��ѿ���();

	uint32_t m_snRecv = 0;
	const uint64_t m_idPlayerGateSession;
	/// <summary>
	/// ��ǰ���������ĸ�Space��
	/// </summary>
	WpSpace m_wpSpace;
	//WpSpace m_wpSpace���˾��鸱��;
	//SpSpace m_spSpace����ս��;
	WpEntity m_wp�ӿ�;
private:
	template<class T_Msg> void RecvMsg(const msgpack::object& obj);
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const Msg��Space& msg);
	void OnRecv(const Msg�뿪Space& msg);
	void OnRecv(const Msg�����˾��鸱��& msg);
	void OnRecv(const Msg��������ս��& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const Msg�ɼ�& msg);
	void OnRecv(const Msg���ر�& msg);
	void OnRecv(const Msg���ر�& msg);
	void OnRecv(const Msg��ѡ& msg);
	void OnRecv(const Msg��Ҹ���ս���б�& msg);
	void OnRecv(const Msg��Ҷ���ս���б�& msg);
	void OnRecv(const Msg��������Ҹ���ս��& msg);
	void OnRecv(const Msg��������Ҷ���ս��& msg);
	void OnRecv(const Msg�л����й��̳�& msg);
	void OnRecv(const Msg����Ի��ѿ���& msg);
	void OnRecv(const Msg�����������λ�ļ����& msg);

	void ѡ�е�λ(std::vector<uint64_t> vecId, const bool b׷�� = false);
	std::vector<WpEntity> Get���й��̳�(��λ���� ����λ����, bool b�����ɼ��еĹ��̳�);

	void Sendѡ����Ч(const Entity& refEntity);

	void Sendѡ�е�λResponce();


	void ForEachSelected(std::function<void(Entity& ref)> fun);
	CoTaskBool Co������������ͼ(WpEntity wp�ӿ�);

	//std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;

	GameSvrSession& m_refGameSvrSession;

	std::vector<uint64_t> m_vecSelectedEntity;
	uint64_t m_id�ϴε�ѡѡ�� = 0;
	std::string m_strNickName;
	uint32_t m_snSend = 0;
	FunCancel m_funCancel����ͼ;
	uint16_t m_idx�л����̳� = 0;
	Position m_pos�ϴε����·Ŀ��;
};

