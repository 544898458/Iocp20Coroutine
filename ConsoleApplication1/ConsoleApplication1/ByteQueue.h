#pragma once
#include<stdint.h>
#include<vector>
#include<WinSock2.h>
#include <tuple> 
/// <summary>
/// 字节队列，用于收发缓存
/// </summary>
class ByteQueue
{
public:
	bool Enqueue(const char* buf, const int len);
	//void DeQueue(char* buf, int &len);
	std::tuple<char*, int> BuildSendBuf();
	void SendComplete(int);
	//int Size();
private:
	std::vector<char> queue;


	std::vector<char> buf;
	std::vector<char>::iterator head = buf.end();

	

};

