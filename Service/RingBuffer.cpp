#include "RingBuffer.h"
#include <malloc.h>
#include <assert.h>
#include <memory.h>

RingBuffer::RingBuffer(int buffSize):m_pData(nullptr)
{
    mTotalSize = buffSize;
    mUseSize = 0;
    mIndex = 0;
    m_pData = (unsigned char*)malloc(buffSize);
    assert(m_pData != nullptr);
}

RingBuffer::~RingBuffer()
{
    if(m_pData != nullptr){
        free(m_pData);
        m_pData = nullptr;
    }
}


bool RingBuffer::Write(int identifier,void* pdata,int dataSize)
{
    //
    int remainSize = mTotalSize - mUseSize;
    //
    if(dataSize > remainSize)  
        return false;
    //
    BufferNode node;
    node.mIndex = mIndex;
    node.mSize = dataSize;
    //
    int dindex = 0;
    do
    {
       int idleSize = mTotalSize - mIndex;
       int cpsize = idleSize >= dataSize ? dataSize : idleSize;
       memcpy(&m_pData[mIndex],((unsigned char*)pdata + dindex),cpsize);
       dindex += cpsize;
       dataSize -= cpsize;
       mIndex = (mIndex += cpsize) % mTotalSize;
    } while (dataSize > 0);

    //
    mUseSize += dataSize;
    //
    mNodeMap[identifier].push_back(node);
}


int RingBuffer::Read(int identifier,void* pdata,int dataBuffSize)
{
    auto iter = mNodeMap.find(identifier);
    if(iter == mNodeMap.end()) 
        return 0;
    //
    if(iter->second.size() <= 0) 
        return 0;
    // 
    int index = iter->second.front().mIndex;
    int size = iter->second.front().mSize;
    //
    size = size > dataBuffSize ? dataBuffSize:size;
    //
    int cpindex = 0;
    int dindex = index;
    int cpRemainSize = size;
    do
    {
        int tmpSize = mTotalSize - dindex;
        int cpsize =  tmpSize > cpRemainSize ? cpRemainSize : tmpSize;
        memcpy((pdata + cpindex),&m_pData[dindex],cpsize);
        cpindex += cpsize;
        dindex =  (dindex += cpsize) % mTotalSize;
        cpRemainSize -=  cpsize;

    } while (cpRemainSize > 0);
    //
    iter->second.pop_front();
    //
    return size;
}
