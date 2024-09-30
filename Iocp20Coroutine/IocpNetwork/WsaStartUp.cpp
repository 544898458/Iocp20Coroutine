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
			LOG(INFO) << "�����µ������ԣ����߼�������";
			break;
		case WSAVERNOTSUPPORTED:
			LOG(INFO) << ("����������");
			break;
		case WSAEINPROGRESS:
			LOG(INFO) << ("����������");
			break;
		case WSAEPROCLIM:
			LOG(INFO) << ("�볢�Թص�����Ҫ���������Ϊ��ǰ���������ṩ������Դ");
			break;
		}

		return false;
	}
}