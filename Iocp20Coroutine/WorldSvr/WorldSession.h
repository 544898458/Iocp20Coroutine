#pragma once
#include"../IocpNetwork/SessionSocketCompletionKey.h"
#include "../LogStrategy/StrategyLog.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../Iocp20Coroutine/MyMsgQueue.h"
class WorldServerAcceptGame;
class WorldSessionFromGame
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGame>;
	WorldSessionFromGame(CompeletionKeySession&) {}
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServerAcceptGame&);
	template<class T>
	void Send(const T& ref)
	{
		ref.msg.sn = (++m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_pSession->Send(buf, len); });
	}
	CompeletionKeySession* m_pSession = nullptr;
	WorldServerAcceptGame* m_pServer = nullptr;
	template<class T> std::deque<T>& GetQueue();
	void Process();
	uint32_t m_snRecv = 0;
	uint32_t m_snSend = 0;
private:
	void OnRecvPack(const void* buf, int len);
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgChangeMoney& msg);
	void OnRecv(const MsgGate转发& msg);
	CoTask<int> Save(const MsgChangeMoney msg);
	CoTask<int> CoLogin(const MsgLogin msg, FunCancel& funCancel);
	MsgQueueMsgPack<WorldSessionFromGame> m_MsgQueue;
	std::deque<MsgGate转发> m_queueGate转发;
	//std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgChangeMoney> m_queueConsumeMoney;

	
	CoTask<int> m_coChangeMoney;

	CoTask<int> m_coLogin;
	FunCancel m_funCancelLogin;
};