#include "ByteQueue.h"
#include<utility>
bool ByteQueue::Enqueue(const char* buf, const int len)
{
    this->queue.insert(this->queue.end(), buf, buf + len);
    return true;
}

//void ByteQueue::DeQueue(char* buf, int &len)
//{
//    len = std::min((int)this->mBuf.size(), len);
//    std::copy(this->mBuf.begin(), this->mBuf.end(), buf);//memcpy替换
//    //return false;
//}

std::tuple<char*, int> ByteQueue:: BuildSendBuf()
{
    if (this->buf.end() != this->head)//固定内存还没发完
    {
        return std::make_tuple(&(*this->head),this->buf.end()-this->head);
    }
        
    auto size = queue.size();
    if (size == 0)//没有可发的数据
        return std::make_tuple(nullptr, 0);

    this->buf.resize(size);//可能换内存
    std::copy(this->queue.begin(), this->queue.end(), buf.begin());//memcpy替换
    //pOverlapped->wsabuf.len = refSize;
    return std::make_tuple(&buf[0], (int)buf.size());
}

void ByteQueue::SendComplete(int sent)
{
    this->head += sent;
}
