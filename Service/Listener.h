#ifndef __LISTENER_H__
#define __LISTENER_H__
#include "EpollIo.h"
#include <sys/types.h>
#include <sys/epoll.h>

class IListeneSink 
{
public:
    virtual void ListeneSink(int fd,uint addr) = 0;
};
//=================================================

class Listener : public IEpEvent {
private:
    int m_fd;
    short m_port;
    epoll_event m_ev;
    EpollIo* m_pEpoll;
    IListeneSink* m_pIListeneSink;
public:
    Listener(short port,EpollIo* pEpoll, IListeneSink* pIListeneSink);
    virtual ~Listener();
public:
    virtual void DoEpEvent(uint evType); 
public:
    bool StartListen();
protected:
    bool SetNonBlock(int fd);
};

#endif