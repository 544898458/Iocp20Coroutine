#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "glog.lib")
#pragma comment(lib, "Mswsock.lib")

#include "../IocpNetwork/ServerTemplate.h"
#include "../IocpNetwork/ListenSocketCompeletionKeyTemplate.h"
#include "../IocpNetwork/SessionSocketCompeletionKeyTemplate.h"
#include "../CoRoutine/CoTimer.h"

class WorldServer;
class WorldSession
{
public:
	void OnInit(Iocp::SessionSocketCompeletionKey<WorldSession>& session, WorldServer&)
	{
		
	}
	int OnRecv(Iocp::SessionSocketCompeletionKey<WorldSession>& refSession, const char buf[], int len) 
	{
		return len;
	}
	void OnDestroy() 
	{

	}
};
class WorldServer 
{
public:
	void OnAdd(Iocp::SessionSocketCompeletionKey<WorldSession>& session)
	{

	}
	
};


template Iocp::Server<WorldServer>;
template bool Iocp::Server<WorldServer>::Init<WorldSession>(const uint16_t);
template void Iocp::ListenSocketCompeletionKey::StartCoRoutine<WorldSession, WorldServer >(HANDLE hIocp, SOCKET socketListen, WorldServer&);
template Iocp::SessionSocketCompeletionKey<WorldSession>;
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
	Iocp::Server<WorldServer> accept;
	accept.WsaStartup();
	accept.Init<WorldSession>(12346);
	
	while (g_running)
	{
		Sleep(100);
		//accept.m_Server.Update();
		CoTimer::Update();
	}
	accept.Stop();
	LOG(INFO) << "WorldSvr正常退出,GetCurrentThreadId=" << GetCurrentThreadId();
}
