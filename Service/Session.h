#ifndef __SESSION_H__
#define __SESSION_H__
#include <sys/types.h>
#include <sys/epoll.h>
#include "EpollIo.h"
#include "ServiceTypeHead.h"

#define SESSIONBUFFSIZE 8192

//=================================================
class NetService;
class RingBuffer;
//=================================================
class Session : public IEpEvent {
    friend NetService;
private:
    //
    short m_idex;
    ushort m_cycle;
    int m_fd;
    int m_addr;
    //
    SessionID m_Sid;
private:
    epoll_event m_ev;
    EpollIo* m_pEpoll;
    RingBuffer* m_pSendBuffer;
public:
    Session(short idex,EpollIo* pEpoll,RingBuffer* pBuff);
    ~Session();
public:
    virtual void DoEpEvent(uint evType); 
public:
    void Attch(int fd,int addr,ushort cycle);
    void Reset();
    bool Send(void* pdata,int size);
protected:
    void DoRecv();
    void DoSend();
};
//=================================================
#endif