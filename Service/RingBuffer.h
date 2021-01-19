#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <map>
#include <list>

struct BufferNode
{
    int mIndex;
    int mSize;
};
typedef std::map<int,std::list<BufferNode>> NodeMap;
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
    bool Write(int identifier,void* pdata,int dataSize);
    int  Read(int identifier,void* pdata,int dataBuffSize);
};

#endif