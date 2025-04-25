#pragma once
#include "../IocpNetwork/WebSocketSession.h"
#include <deque>
#include "../GameSvr/MyMsgQueue.h"
#include "../IocpNetwork/MsgQueueMsgPack.h"
class GateServer;
/// <summary>
/// �Լ��Ƿ������������H5��Ϸ�ͻ���ͨ����WebSocketЭ����������һ������
/// </summary>
class GateSession
{
public:
	using CompeletionKeySession = WebSocketSession<GateSession>;
	GateSession(CompeletionKeySession& ref) :m_refSession(ref)
	{
	}
	uint64_t GetId()const { return (uint64_t)this; }
	//int OnRecv(CompeletionKeySession&, const void* buf, int len);
	/// <summary>
	/// WebSocket�յ�һ�����������ư�
	/// </summary>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	void OnRecvWsPack(const void* buf, const int len);

	void OnDestroy();
	void OnInit(GateServer&);
	void OnRecvWorldSvr(const MsgLoginResponce& msg);
	void OnRecvWorldSvr(const MsgGateDeleteSession& msg);
	/// <summary>
	/// �����߳��У����̣߳�����
	/// </summary>
	bool Process();
	template<class T> std::deque<T>& GetQueue();
	CompeletionKeySession& m_refSession;
	/// <summary>
	/// ���ﱣ��Ķ��ǽ��������Ϣ����,�����л��������̣߳�����������Ϣ���߼��߳�
	/// </summary>
	std::deque<MsgLogin> m_queueLogin;
	std::deque<MsgGateת��> m_queueMsgGateת��;
	MsgQueueMsgPack<GateSession> m_MsgQueue;
	uint32_t m_snRecv = 0;
	uint32_t m_snSendToGameSvr = 0;
	uint32_t m_snSendToWorldSvr = 0;
	uint32_t m_snSendToClient = 0;//������Ϸǰ��
	bool m_bLoginOk = false;
private:
	void OnRecv(const MsgLogin& msg);
	void OnRecv(const MsgGateת��& msg);
	CoTask<int> CoLogin(MsgLogin msg, FunCancel& funCancel);

	CoTask<int> m_coLogin;
	FunCancel m_funCancelLogin;
	bool m_bClosed = false;
};

