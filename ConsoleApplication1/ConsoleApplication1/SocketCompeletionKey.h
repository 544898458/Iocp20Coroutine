#pragma once
#include<WinSock2.h>
#include <mswsock.h>

#include"Define.h"
#include"Overlapped.h"
#include<stdio.h>
#include<assert.h>
#include"CoTask.h"
/// <summary>
	/// 对应一个Socket，可能是监听Socket也可能是连接Socket
	/// 监听Socket同时只有一个重叠操作Accept
	/// 连接Socket同时有两个重叠操作Recv和Accept
	/// </summary>
class SocketCompeletionKey 
{
public:
	SocketCompeletionKey(SOCKET s) :socket(s) {

	}
	virtual void StartCoRoutine() = 0;
protected:
	SOCKET Socket() const;
	void CloseSocket();
public:
	HANDLE hIocp = nullptr;
private:
	SOCKET socket;
	
};

