#pragma once
#include<stdint.h>
#include<vector>
/// <summary>
/// �ֽڶ��У������շ�����
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

