#pragma once
#include<stdint.h>
#include<vector>
#include<WinSock2.h>
#include <tuple> 
#include <mutex>
constexpr int  MAX_RECV_COUNT = 1024;

/// <summary>
/// �ֽڶ��У������շ�����
/// </summary>
class ByteQueue
{
	friend class ByteQueueSend;
	friend class ByteQueueRecv;
public:
	ByteQueue();
	bool Enqueue(const void* buf, const int len);
	//void DeQueue(char* buf, int &len);
	bool Empty();
private:
	std::mutex mutex;

	std::vector<char> queue;

	/// <summary>
	/// �̶��ڴ棬��������޸ģ���ֹ�ڴ��ƶ�
	/// </summary>
	std::vector<char> buf;
	std::vector<char>::iterator head;
};

class ByteQueueSend
{
public:
	std::tuple<char*, int> BuildSendBuf();
	void Complete(int);
	void PopFront(int);

	ByteQueue queue;
};
class ByteQueueRecv
{
public:
	ByteQueueRecv();
	std::tuple<char*, int> BuildRecvBuf();
	std::tuple<char*, int> Complete(int);
	void PopFront(int);
private:
	ByteQueue queue;
};