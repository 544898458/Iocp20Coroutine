#include "pch.h"
#include "WsaStartUp.h"
#include <Winsock2.h>
bool Iocp::WsaStartup()
{
	WORD wdVersion = MAKEWORD(2, 2);
	WSADATA wdScokMsg;
	int nRes = WSAStartup(wdVersion, &wdScokMsg);

	if (0 == nRes)
		return true;
	{
		switch (nRes)
		{
		case WSASYSNOTREADY:
			LOG(INFO) << "重启下电脑试试，或者检查网络库";
			break;
		case WSAVERNOTSUPPORTED:
			LOG(INFO) << ("请更新网络库");
			break;
		case WSAEINPROGRESS:
			LOG(INFO) << ("请重新启动");
			break;
		case WSAEPROCLIM:
			LOG(INFO) << ("请尝试关掉不必要的软件，以为当前网络运行提供充足资源");
			break;
		}

		return false;
	}
}