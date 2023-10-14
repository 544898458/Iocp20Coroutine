#include "ByteQueue.h"
#include<utility>
bool ByteQueue::Enqueue(const uint8_t* buf, const int len)
{
    this->mBuf.insert(this->mBuf.end(), buf, buf + len);
    return true;
}

void ByteQueue::DeQueue( uint8_t* buf, int &len)
{
    len = std::min((int)this->mBuf.size(), len);
    std::copy(this->mBuf.begin(), this->mBuf.end(), buf);//memcpyÌæ»»
    //return false;
}
