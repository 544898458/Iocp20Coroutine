#pragma once
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "../IocpNetwork/WebSocketSession.h"

class MyServer;
class MySession
{
public:
	MySession();
	void OnRecvWsPack(const void* buf, const int len);
	void OnInit(WebSocketSession<MySession>& refWsSession, MyServer& server);
	void OnDestroy();
	template<class T>
	void Send(const T& ref);
	/// <summary>
	/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
	/// </summary>
	std::vector<SpEntity> m_vecSpEntity;
	/// <summary>
	/// ���������Ϣ���У�������Ϣ����ɶ˿��̣߳�������Ϣ�����̣߳�����̨�����̣߳�
	/// </summary>
	MyMsgQueue m_msgQueue;
	MyServer* m_pServer = nullptr;
	std::vector<uint64_t> m_vecSelectedEntity;
	std::string m_nickName;
private:
	template<class T>
	void PushMsg(const msgpack::object& obj);
	WebSocketSession<MySession>* m_pWsSession = nullptr;
};
