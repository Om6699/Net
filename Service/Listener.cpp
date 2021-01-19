#include "Listener.h"
#include "ServiceTypeHead.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <memory.h>
#include <fcntl.h>

Listener::Listener(short port,EpollIo* pEpoll,IListeneSink* pIListeneSink) {
    m_port = port;
    m_pEpoll = pEpoll;
    m_pIListeneSink = pIListeneSink;
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_fd > INVALID_FD) SetNonBlock(m_fd);

}

Listener::~Listener() {
    if(m_fd > INVALID_FD) {
        close(m_fd);
    }
    m_fd = INVALID_FD;
}

bool Listener::SetNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if(-1 == fcntl(fd, F_SETFL, flags))
        return false;
    return true;
}

void Listener::DoEpEvent(uint evType) {
    if(EPOLLIN != evType) return;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));    
    socklen_t cslen = sizeof(sockaddr_in);
    int fd = accept(m_fd,(struct sockaddr*)&addr,&cslen);
    if(m_pIListeneSink != nullptr) {
        if(SetNonBlock(fd)) {
            m_pIListeneSink->ListeneSink(fd,addr.sin_addr.s_addr);
        }
    }
}

bool Listener::StartListen() 
{
    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(m_port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    if(-1 == bind(m_fd, (struct sockaddr *)&saddr, sizeof(saddr))){
        return false;
    }
    if(-1 == listen(m_fd, 128)){
        return false;
    }

    //监听事件
    memset(&m_ev,0,sizeof(epoll_event));
    m_ev.data.fd = m_fd;
    m_ev.data.ptr = this;
    m_ev.events = EPOLLIN;
    //注册到epoll
    m_pEpoll->Add(m_fd, &m_ev);
    return true;
 }
//=================================================