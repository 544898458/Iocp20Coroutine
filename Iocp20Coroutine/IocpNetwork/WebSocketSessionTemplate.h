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
		LOG(INFO) << "收到前端发来的密文,len=" << len;
		const auto i32已处理 = m_SslTls.处理前端发来的密文(buf, len);

		获取准备发往前端的密文并发给前端();

		{
			char buf明文[4096];
			const auto i32读出明文字节 = m_SslTls.读出已解密的明文(buf明文);
			if (0 < i32读出明文字节)
			{
				const auto write = m_webSocketEndpoint->from_wire(buf明文, i32读出明文字节);
				assert(write == i32读出明文字节);
			}
		}
		return i32已处理;
	}
	else
	{
		m_webSocketEndpoint->from_wire(buf, len);
		return len;
	}
}
template<class T_Session>
void WebSocketSession< T_Session>::OnDestroy()
{
	m_Session.OnDestroy();


}
