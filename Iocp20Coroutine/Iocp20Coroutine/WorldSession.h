#pragma once
#include"../IocpNetwork/SessionSocketCompeletionKey.h"
#include "../LogStrategy/StrategyLog.h"
#include "../IocpNetwork/WebSocketSession.h"
class WorldServer;
class WorldSession
{
public:
	using CompeletionKeySession = WebSocketSession<WorldSession>;
	int OnRecvWsPack(const void *buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref) 
	{
		//��ͨ�ṹ���л�����ѹ���޼���
		CHECK_PTR(m_pSession);
		constexpr uint16_t usSize = sizeof(ref);
		m_pSession->Send(&usSize, sizeof(usSize));
		m_pSession->Send(&ref, usSize);
		LOG(INFO) << typeid(T).name();
	}
	CompeletionKeySession* m_pSession = nullptr;
};

