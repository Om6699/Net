#include "EpollIo.h"
#include <sys/epoll.h>
#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include "ServiceTypeHead.h"

//=================================================
EpollIo::EpollIo(int maxEvent)
{
    m_epfd = epoll_create(maxEvent);
    pEvents = (epoll_event*)malloc(sizeof(epoll_event)*maxEvent );
    assert( m_epfd > 0 && pEvents != nullptr);
    m_maxEvent = maxEvent;
}

EpollIo::~EpollIo() 
{
    close(m_epfd);
    m_epfd = INVALID_FD;
    free(pEvents);
    pEvents = nullptr;
}

int EpollIo::Mod(int fd,epoll_event* ev)
{
    return epoll_ctl(m_epfd,EPOLL_CTL_MOD,fd,ev);
}

int EpollIo::Add(int fd,epoll_event* ev)
{
    return epoll_ctl(m_epfd,EPOLL_CTL_ADD,fd,ev);
}

int EpollIo::Del(int fd)
{
    return epoll_ctl(m_epfd,EPOLL_CTL_DEL,fd,nullptr);
}

void EpollIo::Wait(int timeOut)
{
    int count = epoll_wait(m_epfd,pEvents,m_maxEvent,timeOut);
    for (int i = 0; i < count; i++) {
        ((IEpEvent*)pEvents[i].data.ptr)->DoEpEvent(pEvents[i].events);
    }
}
//=================================================