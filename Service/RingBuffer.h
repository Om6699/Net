#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <map>
#include <list>

struct BufferNode
{
    int mIndex;
    int mSize;
};
typedef std::map<unsigned long long,std::list<BufferNode>> NodeMap;
class RingBuffer
{
private:
    unsigned char* m_pData;
    int mIndex;
    int mUseSize;
    int mTotalSize;
private:
    NodeMap mNodeMap;
public:
    RingBuffer(int buffSize);
    ~RingBuffer();
public:
    bool Write(unsigned long long identifier,void* pdata,int dataSize);
    int  Read(unsigned long long identifier,void* pdata,int dataBuffSize);
};

#endif