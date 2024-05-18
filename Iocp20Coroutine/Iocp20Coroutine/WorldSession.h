#pragma once
#include"../IocpNetwork/SessionSocketCompeletionKey.h"
#include "../LogStrategy/StrategyLog.h"
class WorldServer;
class WorldSession
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey<WorldSession>;
	int OnRecv(CompeletionKeySession& refSession, const char buf[], int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref) 
	{
		//普通结构序列化，无压缩无加密
		CHECK_PTR(m_pSession);
		m_pSession->Send(&ref, sizeof(ref));
		LOG(INFO) << typeid(T).name();
	}
	CompeletionKeySession* m_pSession = nullptr;
};

