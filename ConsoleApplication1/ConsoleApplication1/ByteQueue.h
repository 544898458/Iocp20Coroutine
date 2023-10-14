#pragma once
#include<stdint.h>
#include<vector>
/// <summary>
/// 字节队列，用于收发缓存
/// </summary>
class ByteQueue
{
public:
	bool Enqueue(const uint8_t* buf, const int len);
	void DeQueue(uint8_t* buf, int &len);
	int Size();
private:
	std::vector<uint8_t> mBuf;
};

