#pragma once
#include<vector>
#define MAX_RECV_COUNT  1024

namespace Iocp 
{
	/// <summary>
	/// 重叠操作，对应一次Accet、Recv、Send
	/// CONTAINING_RECORD（）可以根据Overlapped获取PER_IO_CONTEXT数据类型。前提是Overlapped是PER_IO_CONTEXT的第一个成员。CONTAINING_RECORD可以根据Overlapped的地址获取其所在PER_IO_CONTEXT结构体的地址指针。
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
	/// 对应一个Socket，可能是监听Socket也可能是连接Socket
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