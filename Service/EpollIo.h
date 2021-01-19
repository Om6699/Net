#ifndef __EPOLLIO_H__
#define __EPOLLIO_H__
#include <sys/types.h>
//===================================================
class IEpEvent{
    public:
        virtual void DoEpEvent(uint evType) = 0; 
};
//===================================================
class NetService;
struct epoll_event;
class EpollIo
{
    friend NetService;
private:
    int m_epfd;
    epoll_event* pEvents;
    int m_maxEvent;
public:
    EpollIo(int maxEvent);
    ~EpollIo();
public:
    int GetEpFd(){ return m_epfd; }
    int Mod(int fd,epoll_event* ev);
    int Add(int fd,epoll_event* ev);
    int Del(int fd);
protected:
    void Wait(int timeOut);
};
#endif