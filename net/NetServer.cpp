#include "NetServer.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <memory.h>
#include <fcntl.h>
#include <assert.h>
#include <malloc.h>

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

void Listener::DoEpEvent(int evType) {
    if(EPOLLIN != evType) return;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));    
    socklen_t cslen = sizeof(sockaddr_in);
    int fd = accept(m_fd,(struct sockaddr*)&addr,&cslen);
    if(m_pIListeneSink != nullptr) {
        if(SetNonBlock(fd)) {
            m_pIListeneSink->Listene(fd,addr.sin_addr.s_addr);
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
Session::Session(short idex,EpollIo* pEpoll)
{
    m_idex = idex;
    m_fd = INVALID_FD;
    m_cycle = 0;
    m_addr = 0;
    m_pEpoll = pEpoll;
}


Session::~Session() 
{
    if(m_fd != INVALID_FD)
        close(m_fd);
}


void Session::DoEpEvent(int evType)
{
    if((evType & EPOLLIN)) {
        DoRecv();
    }
    else if((evType & EPOLLOUT)) {
        DoSend();
    }
    else {
    }
}


void Session::Attch(int fd,int addr,short cycle)
{
    m_fd = fd;
    m_addr = addr;
    m_cycle = cycle;

    //设置监听事件
    memset(&m_ev,0,sizeof(epoll_event));
    m_ev.data.fd = m_fd;
    m_ev.data.ptr = this;
    m_ev.events = EPOLLIN;
    //注册到epoll
    m_pEpoll->Add(m_fd,&m_ev);
}

void Session::Reset()
{
    //删除epoll
    m_pEpoll->Del(m_fd);
    //
    close(m_fd);
    m_fd = INVALID_FD;
    m_addr = 0;
    m_cycle = 0;
}


bool Session::Send(void* buf,int size) {
    //数据缓存
    memcpy(m_sendbuf,buf,size);
    m_size = size;
    //修改监听事件
    if(!(m_ev.events & EPOLLOUT)) 
    {
        m_ev.events |= EPOLLOUT;
        m_pEpoll->Mod(m_fd,&m_ev);
    }
    return true;
}


void Session::DoRecv() {
    char buff[1024];
    int size = recv(m_fd,buff,sizeof(buff),0);
    if(size <= 0){
        Reset();
        return;
    }
    Send(buff,size);
}

void Session::DoSend() {
    write(m_fd,m_sendbuf,m_size);
    //若无数据可发修改监听事件为只读
    if(m_ev.events & EPOLLOUT) 
    {
        m_ev.events &= ~EPOLLOUT;
        m_pEpoll->Mod(m_fd,&m_ev);
    }
}
//=================================================
NetServer::NetServer(short poolSize,int port):m_Io(poolSize),m_pListener(port,&m_Io,this){
    for (short i = 0; i < poolSize; i++) {
        m_SessionVec.push_back(new Session(i,&m_Io));
    }
    m_Cycle = 0;
}

NetServer::~NetServer() 
{

}

void NetServer::Listene(int fd,int addr)
{
    Session* pSession = nullptr;
    for (size_t i = 0; i < m_SessionVec.size(); i++) {
        if(m_SessionVec[i]->m_fd != INVALID_FD) 
            continue;
        pSession = m_SessionVec[i];
        break;
    }
    if(pSession == nullptr) 
        return;
    pSession->Attch(fd,addr,m_Cycle);
    //循环计数
    m_Cycle = (++m_Cycle) % INT16_MAX;
}


void NetServer::run(int timeOut)
{
    m_pListener.StartListen();
    for(;;) {
        m_Io.Wait(timeOut);
    }
}
