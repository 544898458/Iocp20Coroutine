#pragma once
#include<vector>
namespace Iocp 
{
	/// <summary>
	/// CONTAINING_RECORD（）可以根据Overlapped获取PER_IO_CONTEXT数据类型。前提是Overlapped是PER_IO_CONTEXT的第一个成员。CONTAINING_RECORD可以根据Overlapped的地址获取其所在PER_IO_CONTEXT结构体的地址指针。
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