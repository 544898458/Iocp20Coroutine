#pragma once
#include"../IocpNetwork/SessionSocketCompletionKey.h"
#include "../LogStrategy/StrategyLog.h"
//#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
#include "../Iocp20Coroutine/MyMsgQueue.h"
class WorldSvrAcceptGate;
class WorldSessionFromGate
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompletionKey<WorldSessionFromGate>;
	WorldSessionFromGate(CompeletionKeySession&ref):m_refSession(ref) {}
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(WorldSvrAcceptGate&);
	template<class T>
	void Send(const T& ref)
	{
		ref.msg.sn = (++m_snSend);
		MsgPack::SendMsgpack(ref, [this](const void* buf, int len) { this->m_pSession->Send(buf, len); });
	}
	CompeletionKeySession* m_pSession = nullptr;
	WorldSvrAcceptGate* m_pServer = nullptr;
	template<class T> std::deque<T>& GetQueue();
	bool Process();
	uint32_t m_snRecv = 0;
	uint32_t m_snSend = 0;

	CompeletionKeySession& m_refSession;
private:
	void OnRecvPack(const void* buf, int len);
	void OnRecv(const MsgGate转发& msg);
	
	MsgQueueMsgPack<WorldSessionFromGate> m_MsgQueue;
	std::deque<MsgGate转发> m_queueGate转发;
	
	CoTask<int> m_coLogin;
	FunCancel m_funCancelLogin;
};