#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"Overlapped.h"
#include<stdio.h>
#include<assert.h>
#include"CoRoutine/CoTask.h"
/// <summary>
	/// ��Ӧһ��Socket�������Ǽ���SocketҲ����������Socket
	/// ����Socketͬʱֻ��һ���ص�����Accept
	/// ����Socketͬʱ�������ص�����Recv��Accept
	/// ���ִ���CompeletionKey ˵�����Ǵ���CreateIoCompletionPort�Ĳ������ڳ��ֶ�Ӧ�¼������
	/// </summary>
class SocketCompeletionKey 
{
public:
	SocketCompeletionKey(SOCKET s) :socket(s) 
	{

	}
	/// <summary>
	/// ����Socketͬʱֻ��һ���ص�����Accept
	/// ����Socketͬʱ�������ص�����Recv��Accept
	/// </summary>
	virtual void StartCoRoutine() = 0;
protected:
	SOCKET Socket() const;
	void CloseSocket();
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;
	
};

