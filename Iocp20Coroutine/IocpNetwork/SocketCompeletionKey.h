#pragma once
#include<WinSock2.h>
#include<mswsock.h>

#include"./Overlapped.h"
#include<assert.h>
#include"../CoRoutine/CoTask.h"
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
	//virtual void StartCoRoutine() = 0;
	void CloseSocket();
protected:
	SOCKET Socket() const;
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;

};

