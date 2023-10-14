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

std::tuple<char*, int> ByteQueue:: BuildSendBuf()
{
    if (this->buf.end() != this->head)//�̶��ڴ滹û����
    {
        return std::make_tuple(&(*this->head),this->buf.end()-this->head);
    }
        
    auto size = queue.size();
    if (size == 0)//û�пɷ�������
        return std::make_tuple(nullptr, 0);

    this->buf.resize(size);//���ܻ��ڴ�
    std::copy(this->queue.begin(), this->queue.end(), buf.begin());//memcpy�滻
    this->queue.clear();
    this->head = this->buf.begin();
    return std::make_tuple(&(*this->head), (int)buf.size());
}

std::tuple<char*, int> ByteQueue::BuildRecvBuf()
{
    if (this->buf.end() != this->head)//�̶��ڴ滹û����
    {
        return std::make_tuple(&(*this->head), this->buf.end() - this->head);
    }

    this->queue.insert(this->queue.end(),this->buf.begin(), this->buf.end());//memcpy�滻

    this->head = this->buf.begin();
    return std::make_tuple(&(*this->head), (int)buf.size());
}
void ByteQueue::Complete(int sent)
{
    this->head += sent;
}
