#pragma once
#include<vector>
namespace Iocp 
{
	/// <summary>
	/// CONTAINING_RECORD�������Ը���Overlapped��ȡPER_IO_CONTEXT�������͡�ǰ����Overlapped��PER_IO_CONTEXT�ĵ�һ����Ա��CONTAINING_RECORD���Ը���Overlapped�ĵ�ַ��ȡ������PER_IO_CONTEXT�ṹ��ĵ�ַָ�롣
	///	PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(lpOverlapped, PER_IO_CONTEXT, m_Overlapped);
	/// </summary>
	struct MyOverlapped
	{
		OVERLAPPED overlapped;
		SOCKET socket;
		enum Op 
		{
			Accept,
		};
		Op op;
	};
	struct CompeletionKey {

	};
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		bool PostAccept();
		static DWORD WINAPI ThreadProc(LPVOID lpParameter);
		bool PostSend(SOCKET socket);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocp = nullptr;
		std::vector<HANDLE> vecThread;
	};

}