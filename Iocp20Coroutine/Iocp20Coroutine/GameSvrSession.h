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
	/// WebSocket�յ�һ�����������ư�
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
	/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
	/// </summary>
	std::set<SpEntity> m_vecSpEntity;

	GameSvr* m_pServer = nullptr;
	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;

	template<class T>
	std::deque<T>& GetQueue();

	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	void Process();
	std::vector<CoTask<int>>	m_vecCoRpc;
	std::vector<std::shared_ptr<FunCancel>>	m_vecFunCancel;
	bool m_bLoginOk = false;
private:
	void OnRecvPack(const void* buf, const int len);
	/// <summary>
	/// ���߼��̣߳�����̨�����̣߳�����
	/// </summary>
	/// <param name="msg"></param>
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgMove& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgSelectRoles& msg);
	void OnRecv(const MsgAddRole& msg);
	void OnRecv(const MsgAddBuilding& msg);
	void OnRecv(const MsgGateת��& msg);
	template<class T_Msg> void RecvMsg(msgpack::object& obj);


	CoTask<int> CoAddRole();
	CoTask<int> CoAddBuilding();
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgMove> m_queueMove;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgSelectRoles> m_queueSelectRoles;
	std::deque<MsgAddRole> m_queueAddRole;
	std::deque<MsgAddBuilding> m_queueAddBuilding;
	std::deque<MsgGateת��> m_queueGateת��;

	MsgQueueMsgPack<GameSvrSession> m_MsgQueue;
private:
	WebSocketGameSession* m_pWsSession = nullptr;
};
