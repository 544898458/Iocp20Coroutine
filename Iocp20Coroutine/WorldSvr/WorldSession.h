#pragma once
#include"../IocpNetwork/SessionSocketCompletionKey.h"
#include "../LogStrategy/StrategyLog.h"
#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../Iocp20Coroutine/MyMsgQueue.h"
class WorldServer;
class WorldSession
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey < WorldSession >;
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref) 
	{
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_pSession->Send(buf, len); });
	}
	CompeletionKeySession* m_pSession = nullptr;
	WorldServer* m_pServer = nullptr;
	template<class T>
	std::deque<T>& GetQueue();
	void Process();
private:
	void OnRecvPack(const void* buf, int len);
	void OnRecv(const MsgSay& msg);
	void OnRecv(const MsgConsumeMoney& msg);

	MsgQueueMsgPack<WorldSession> m_MsgQueue;
	std::deque<MsgSay> m_queueSay;
	std::deque<MsgConsumeMoney> m_queueConsumeMoney;
};

