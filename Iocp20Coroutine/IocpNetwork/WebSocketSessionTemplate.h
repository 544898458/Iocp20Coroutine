#include "WebSocketSession.h"

template<class T_Session>
template<class T_Server>
	requires requires(WebSocketSession<T_Session>& refWsSesson, T_Session& refSession, T_Server& server)
{
	//refSession.OnRecvWsPack((const char[])0, (const int )0	);
	refSession.OnInit(server);
}
void WebSocketSession<T_Session>::OnInit(T_Server& server)
{
	//std::lock_guard lock(g_setSessionMutex<WebSocketSession<T_Session> >);
	m_webSocketEndpoint.reset(new MyWebSocketEndpoint<T_Session, WebSocketSession<T_Session> >(&this->m_Session, this));

	this->m_Session.OnInit(server);
	m_SslTls.Init(true);
}
template<class T_Session>
int WebSocketSession<T_Session>::OnRecv(Iocp::SessionSocketCompletionKey<WebSocketSession<T_Session>>& refSession, const void* buf, int len)
{
	if (m_bSslTls)
	{
		LOG(INFO) << "�յ�ǰ�˷���������,len=" << len;
		const auto i32�Ѵ��� = m_SslTls.����ǰ�˷���������(buf, len);

		��ȡ׼������ǰ�˵����Ĳ�����ǰ��();

		{
			char buf����[4096] = { 0 };
			const auto i32���������ֽ� = m_SslTls.�����ѽ��ܵ�����(buf����);
			if (0 < i32���������ֽ�)
			{
				const auto write = m_webSocketEndpoint->from_wire(buf����, i32���������ֽ�);
				LOG_IF(WARNING, write != i32���������ֽ�) << "i32���������ֽ�" << i32���������ֽ� << ",���Ĵ���WS,write=" << write << ",buf����=" << buf����;
				//if(write == i32���������ֽ�)
					//return i32�Ѵ���;
			}
		}
		return i32�Ѵ���;
	}
	else
	{
		const auto write = m_webSocketEndpoint->from_wire(buf, len);
		LOG_IF(WARNING, write != len) << "len" << len << ",���Ĵ���WS,write=" << write << ",buf=" << buf;
		return len;
	}
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();


}
