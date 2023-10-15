#include "ByteQueue.h"
#include<utility>
ByteQueue::ByteQueue()
{
    this->buf.resize(MAX_RECV_COUNT);
    this->head = buf.end();
}
bool ByteQueue::Enqueue(const char* buf, const int len)
{
    this->queue.insert(this->queue.end(), buf, buf + len);
    return true;
}

//void ByteQueue::DeQueue(char* buf, int &len)
//{
//    len = std::min((int)this->mBuf.size(), len);
//    std::copy(this->mBuf.begin(), this->mBuf.end(), buf);//memcpy�滻
//    //return false;
//}

std::tuple<char*, int> ByteQueueSend:: BuildSendBuf()
{
    auto& refQueue = this->queue;
    if (refQueue.buf.end() != refQueue.head)//�̶��ڴ滹û����
    {
        return std::make_tuple(&(*refQueue.head),refQueue.buf.end()-refQueue.head);
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
    auto& refQueue = this->queue;
    if (refQueue.buf.end() != refQueue.head)//�̶��ڴ滹û����
    {
        return std::make_tuple(&(*refQueue.head), refQueue.buf.end() - refQueue.head);
    }

    refQueue.queue.insert(refQueue.queue.end(),refQueue.buf.begin(), refQueue.buf.end());//memcpy�滻

    refQueue.head = refQueue.buf.begin();
    return std::make_tuple(&(*refQueue.head), (int)refQueue.buf.size());
}
void ByteQueueSend::Complete(int sent)
{
    auto& refQueue = this->queue;
    refQueue.head += sent;
}
std::tuple<char*, int>  ByteQueueRecv::Complete(int sent)
{
    auto& refQueue = this->queue;
    refQueue.head += sent;

    refQueue.queue.insert(refQueue.queue.end(), refQueue.buf.begin(), refQueue.head);//memcpy�滻
    refQueue.head = refQueue.buf.begin();
    return std::make_tuple(&(*refQueue.head), (int)refQueue.buf.size());
}

void ByteQueueRecv::PopFront(int len) 
{
    auto& refQueue = this->queue.queue;
    refQueue.erase(refQueue.begin(), refQueue.begin() + len);
}