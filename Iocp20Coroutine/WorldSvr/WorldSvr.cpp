#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")

#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/ThreadPool.h"
#include "../CoRoutine/CoTimer.h"
#include <glog/logging.h>
#include <msgpack.hpp>

enum MsgId;
class WorldClient;
class WorldClientSession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<WorldClientSession>& session, WorldClient&)
	{

	}

	/// <summary>
	///
	/// </summary>
	/// <param name="refSession"></param>
	/// <param name="buf"></param>
	/// <param name="len"></param>
	/// <returns>返回已处理的字节数，这些数据将立刻从接受缓冲中删除</returns>
	int OnRecv(Iocp::SessionSocketCompeletionKey<WorldClientSession>& refSession, const char buf[], int len)
	{
		uint16_t usPackLen(0);
		const auto sizeofPackLen = sizeof(usPackLen);
		if (sizeofPackLen > len)
			return 0;

		usPackLen = *(uint16_t*)buf;
		if (usPackLen > 8192)
		{
			LOG(ERROR) << "跳过数据包len=" << len;
			return len;
		}
		if (usPackLen + sizeofPackLen > len)
			return 0;

		OnRecvPack(buf + sizeofPackLen, len - sizeofPackLen);
		return len;
	}
	void OnRecvPack(const char buf[], int len)
	{
		msgpack::object_handle oh = msgpack::unpack(buf, len);//没判断越界，要加try
		msgpack::object obj = oh.get();
		const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
		LOG(INFO) << obj;
		
		//auto pSessionSocketCompeletionKey = this->m_pWsSession->m_pSession;
		//switch (msgId)
		//{
		//case MsgId::Login:
		//{
		//	const auto msg = obj.as<MsgLogin>();
		//	pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
		//}
		//break;
		//case MsgId::Move:
		//{
		//	const auto msg = obj.as<MsgMove>();
		//	pSessionSocketCompeletionKey->Session.m_Session.m_msgQueue.Push(msg);
		//}
		//break;
		//}
	}
	void OnDestroy()
	{

	}
};
class WorldClient
{
public:
	void OnAdd(Iocp::SessionSocketCompeletionKey<WorldClientSession>& session)
	{

	}

};


template Iocp::Server<WorldClient>;
template bool Iocp::Server<WorldClient>::Init<WorldClientSession>(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WorldClientSession, WorldClient >(HANDLE hIocp, SOCKET socketListen, WorldClient&);
template Iocp::SessionSocketCompeletionKey<WorldClientSession>;
BOOL g_running = TRUE;
BOOL WINAPI fun(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_CLOSE_EVENT://控制台关闭（点右上角X关闭）
		LOG(WARNING) << "不能点右上角X关闭，可能有数据没保存";
		g_running = FALSE;
		Sleep(3000);
		_CrtDumpMemoryLeaks();	 //显示内存泄漏报告
		break;
	case CTRL_C_EVENT:
		g_running = FALSE;
		break;
	}
	return TRUE;
}

int main()
{
	Iocp::ThreadPool::Init();
	Iocp::Server<WorldClient> accept(Iocp::ThreadPool::GetIocp());
	accept.WsaStartup();
	accept.Init<WorldClientSession>(12346);
	accept.Connect( L"127.0.0.1", L"12345");
	while (g_running)
	{
		Sleep(100);
		//accept.m_Server.Update();
		CoTimer::Update();
	}
	accept.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
