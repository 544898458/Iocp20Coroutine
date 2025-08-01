#pragma once
#include"../IocpNetwork/SessionSocketCompletionKey.h"
#include "../LogStrategy/StrategyLog.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../GameSvr/MyMsgQueue.h"
class WorldSvrAcceptGame;
class WorldSessionFromGame
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
	WorldSessionFromGame(CompeletionKeySession&ref):m_refSession(ref), m_funCancelLogin("WorldSessionFromGame.m_funCancelLogin"){}
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(WorldSvrAcceptGame&);
	template<class T>
	void Send(const T& ref)
	{
		_ASSERT(ref.msg.id != MsgId::MsgId_Invalid_0);

		ref.msg.sn = (++m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_refSession.Send(buf, len); });
	}
	//CompeletionKeySession* m_pSession = nullptr;
	WorldSvrAcceptGame* m_pServer = nullptr;
	template<class T> std::deque<T>& GetQueue();
	bool Process();
	uint8_t m_snRecv = 0;
	uint8_t m_snSend = 0;
	CompeletionKeySession& m_refSession;
private:
	void OnRecvPack(const void* buf, int len);
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgChangeMoney& msg, const uint64_t idGateSessionId);
	void OnRecv(const MsgGate转发& msg);
    void OnRecv(const Msg战局结束& msg);
	void OnRecv(const Msg击杀& msg);
	
	CoTask<int> CoChangeMoney(const MsgChangeMoney msg, const std::string& strNickName);
	CoTask<int> CoLogin(const MsgLogin msg, FunCancel& funCancel);
	CoTaskBool Co收到聊天(MsgSay msg);
	CoTaskBool Co战局结束(Msg战局结束 msg);
	CoTaskBool Co击杀(Msg击杀 msg);
	
	MsgQueueMsgPack<WorldSessionFromGame> m_MsgQueue;
	
	//std::deque<MsgSay> m_queueSay;
	//std::deque<MsgChangeMoney> m_queueConsumeMoney;
	std::deque<MsgGate转发> m_queueGate转发;
	std::deque<Msg战局结束> m_queue战局结束;
	std::deque<Msg击杀> m_queue击杀;

	//CoTask<int> m_coChangeMoney;

	CoTask<int> m_coLogin;
	FunCancel安全 m_funCancelLogin;
};