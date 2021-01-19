#ifndef __NETSERVICE_H__
#define __NETSERVICE_H__
#include <sys/types.h>
#include <vector>
#include "EpollIo.h"
#include "Listener.h"
#include "RingBuffer.h"
#include "ServiceTypeHead.h"

class Session;
typedef std::vector<Session*> SessionVec;
//

class INetServiceSink
{
public:
    virtual void NetConnection(Session sid,uint addr) = 0;
    virtual void NetDisConnection(Session sid) = 0;
    virtual void NetRecv(Session sid,void* pdata,int size) = 0;
};

class NetService : public IListeneSink 
{
    EpollIo    m_Io;
    Listener   m_Listener;
    RingBuffer m_Buff;
    SessionVec m_SessionVec;
    ushort     m_Cycle;
public:
    NetService(short poolSize,int port);
    ~NetService();
public:
    virtual void ListeneSink(int fd,uint addr);
public:
    bool SendData(SessionID sid,void* pdata,int size);
    void run(int timeOut = -1);
};

#endif