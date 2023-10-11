#pragma once
#include<vector>
#define MAX_RECV_COUNT  1024

namespace Iocp 
{
	/// <summary>
	/// �ص���������Ӧһ��Accet��Recv��Send
	/// CONTAINING_RECORD�������Ը���Overlapped��ȡPER_IO_CONTEXT�������͡�ǰ����Overlapped��PER_IO_CONTEXT�ĵ�һ����Ա��CONTAINING_RECORD���Ը���Overlapped�ĵ�ַ��ȡ������PER_IO_CONTEXT�ṹ��ĵ�ַָ�롣
	///	PER_IO_CONTEXT* pIoContext = CONTAINING_RECORD(lpOverlapped, PER_IO_CONTEXT, m_Overlapped);
	/// </summary>
	struct MyOverlapped
	{
		enum Op
		{
			Accept,
			Recv,
			Send,
		};
		MyOverlapped(MyOverlapped::Op opInit)
		{
			this->op = opInit;
		}
		OVERLAPPED overlapped = { 0 };

		Op op;
		SOCKET socket;
	};
	/// <summary>
	/// ��Ӧһ��Socket�������Ǽ���SocketҲ����������Socket
	/// </summary>
	struct MyCompeletionKey {
		SOCKET socket;
		char recv_buf[MAX_RECV_COUNT];
	};
	class Accept
	{
	public:
		bool WsaStartup();
		bool Init();
	private:
		bool PostAccept();
		static DWORD WINAPI ThreadProc(LPVOID lpParameter);
		bool PostSend(MyCompeletionKey* pKey, MyOverlapped* pOverlapped);
		bool PostRecv(MyCompeletionKey* pKey);
		//bool flag = true;
	private:
		SOCKET socketAccept=NULL;
		HANDLE hIocp = nullptr;
		std::vector<HANDLE> vecThread;
	};

}