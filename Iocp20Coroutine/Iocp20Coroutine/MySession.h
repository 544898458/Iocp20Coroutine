#pragma once
#include "./IocpNetwork/SessionSocketCompeletionKey.h"
#include "MsgQueue.h"
#include "Entity.h"
class MyWebSocketEndpoint;
/// <summary>
/// ��Ӧһ����������
/// </summary>
class WebSocketSession
{
public:
	/// <summary>
	/// ���캯����OnInit������ŵ��ã���ʼ�������������Ҹ��ط�д��ֻҪ�ܱ���ͨ����Ч������ȫ��ͬ
	/// </summary>
	WebSocketSession();
	void OnInit(Iocp::SessionSocketCompeletionKey<WebSocketSession>& refSession);
	/// <summary>
	/// �û��Զ��庯���������Ǵ����ݣ���������û�У������WebSocketЭ�鸺��Ĺ���
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	int OnRecv(Iocp::SessionSocketCompeletionKey<WebSocketSession> &refSession,const char buf[], int len);
	/// <summary>
/// ��ȫ������set��ɾ�����ӣ���ȫ��Space��ɾ��ʵ��
/// </summary>
	void OnDestroy();

	/// <summary>
	/// ������Ϣ���ͻ���
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="ref"></param>
	template<class T> void Send(const T& ref);


	/// <summary>
	/// ���������Ϣ���У�������Ϣ����ɶ˿��̣߳�������Ϣ�����̣߳�����̨�����̣߳�
	/// </summary>
	MsgQueue m_msgQueue;
	/// <summary>
	/// ��ԴWebSocket��
	/// </summary>
	std::unique_ptr<MyWebSocketEndpoint> m_webSocketEndpoint;
	/// <summary>
	/// ����Space�ռ��ʵ�壨��ҽ�ɫ��
	/// </summary>
	Entity m_entity;
private:
	Iocp::SessionSocketCompeletionKey<WebSocketSession> *m_pSession;
};


