#include "pch.h"
#include "ByteQueue.h"
#include<utility>
ByteQueue::ByteQueue()
{
}
bool ByteQueue::Enqueue(const void* buf, const int len)
{
	std::lock_guard lock(mutex);

	this->queue.insert(this->queue.end(), (const char*)buf, (const char*)buf + len);
	return true;
}

bool ByteQueue::Empty()
{
	std::lock_guard lock(mutex);
	return this->queue.empty();
}

//void ByteQueue::DeQueue(char* buf, int &len)
//{
//    len = std::min((int)this->mBuf.size(), len);
//    std::copy(this->mBuf.begin(), this->mBuf.end(), buf);//memcpy�滻
//    //return false;
//}

std::tuple<char*, int> ByteQueueSend::BuildSendBuf()
{
	std::lock_guard lock(queue.mutex);
	auto& refQueue = this->queue;
	if (!refQueue.buf.empty() && refQueue.buf.end() != refQueue.head)//�̶��ڴ滹û����
	{
		return std::make_tuple(&(*refQueue.head), (int)(refQueue.buf.end() - refQueue.head));
	}

	auto size = refQueue.queue.size();
	if (size == 0)//û�пɷ�������
		return std::make_tuple(nullptr, 0);

	refQueue.buf.resize(size);//���ܻ��ڴ�
	std::copy(refQueue.queue.begin(), refQueue.queue.end(), refQueue.buf.begin());//memcpy�滻
	refQueue.queue.clear();
	refQueue.head = refQueue.buf.begin();
	return std::make_tuple(&(*refQueue.head), (int)refQueue.buf.size());
}

std::tuple<char*, int> ByteQueueRecv::BuildRecvBuf()
{
	std::lock_guard lock(queue.mutex);
	auto& refQueue = this->queue;
	if (refQueue.buf.end() != refQueue.head)//�̶��ڴ滹û����
	{
		return std::make_tuple(&(*refQueue.head), (int)(refQueue.buf.end() - refQueue.head));
	}

	refQueue.queue.insert(refQueue.queue.end(), refQueue.buf.begin(), refQueue.buf.end());//memcpy�滻

	refQueue.head = refQueue.buf.begin();
	return std::make_tuple(&(*refQueue.head), (int)refQueue.buf.size());
}

int ByteQueueSend::Complete(int sent)
{
	std::lock_guard lock(queue.mutex);
	auto& refQueue = this->queue;
	CHECK_RET_DEFAULT(refQueue.head + sent <= refQueue.buf.end());
	refQueue.head += sent;
	return int(refQueue.buf.end() - refQueue.head + refQueue.queue.size());
}
ByteQueueRecv::ByteQueueRecv()
{
	std::lock_guard lock(queue.mutex);
	queue.buf.resize(MAX_RECV_COUNT);
	this->queue.head = queue.buf.begin();
}
std::tuple<char*, int> ByteQueueRecv::Complete(int sent)
{
	std::lock_guard lock(queue.mutex);
	auto& refQueue = this->queue;
	refQueue.head += sent;

	refQueue.queue.insert(refQueue.queue.end(), refQueue.buf.begin(), refQueue.head);//memcpy�滻
	refQueue.head = refQueue.buf.begin();
	return std::make_tuple(&(*refQueue.queue.begin()), (int)refQueue.queue.size());
}

void ByteQueueRecv::PopFront(int len)
{
	std::lock_guard lock(queue.mutex);
	auto& refQueue = this->queue.queue;
	refQueue.erase(refQueue.begin(), refQueue.begin() + len);
}