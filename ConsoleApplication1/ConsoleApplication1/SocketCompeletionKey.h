#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"MyOverlapped.h"
#include<stdio.h>
#include<assert.h>
#include"CoTask.h"
/// <summary>
	/// ��Ӧһ��Socket�������Ǽ���SocketҲ����������Socket
	/// ����Socketͬʱֻ��һ���ص�����Accept
	/// ����Socketͬʱ�������ص�����Recv��Accept
	/// </summary>
class SocketCompeletionKey 
{
public:
	SocketCompeletionKey(SOCKET s) :socket(s) {

	}
	virtual void StartCoRoutine() = 0;
protected:
	SOCKET Socket() const { return socket; }
private:
	//virtual bool PostSend( MyOverlapped* pOverlapped) = 0;
	//virtual bool PostRecv(MyOverlapped* pOverlapped) = 0;
	//virtual void OnCompleteRecv(const DWORD number_of_bytes, MyOverlapped* pOverlapped) = 0;
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;
	
};

