#include "Session.h"
#include <unistd.h>
#include <memory.h>
#include <sys/socket.h>
#include "RingBuffer.h"

Session::Session(short idex,EpollIo* pEpoll,RingBuffer* pBuff)
{
    m_idex = idex;
    m_fd = INVALID_FD;
    m_cycle = 0;
    m_addr = 0;
    m_pEpoll = pEpoll;
    m_pSendBuffer = pBuff;
    m_Sid.sid = INVALID_SESSIONID;
}


Session::~Session() 
{
    if(m_fd != INVALID_FD)
        close(m_fd);
}

void Session::DoEpEvent(uint evType)
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

void Session::Attch(int fd,int addr,ushort cycle)
{
    m_fd = fd;
    m_addr = addr;
    m_cycle = cycle;
    //
    m_Sid.sct.fd = m_fd;
    m_Sid.sct.index = m_idex;
    m_Sid.sct.cycle = m_cycle;

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
    //
    m_Sid.sid = INVALID_SESSIONID;

}

bool Session::Send(void* pdata,int size) 
{
    //数据缓存
    if(m_pSendBuffer->Write(m_Sid.sid,pdata,size))
    {
        //修改监听事件
        if(!(m_ev.events & EPOLLOUT)) 
        {
            m_ev.events |= EPOLLOUT;
            m_pEpoll->Mod(m_fd,&m_ev);
        }
        return true;
    }
    return false;
}


void Session::DoRecv() {
    char buff[SESSIONBUFFSIZE];
    int size = recv(m_fd,buff,sizeof(buff),0);
    if(size <= 0){
        Reset();
        return;
    }
}

void Session::DoSend() 
{
    char buff[SESSIONBUFFSIZE]={0};
    int size = m_pSendBuffer->Read(m_Sid.sid,(void*)buff,sizeof(buff));
    if(size > 0)
    {
        write(m_fd,buff,size);
    }
    else
    {
        //若无数据可发修改监听事件为只读
        if(m_ev.events & EPOLLOUT) 
        {
            m_ev.events &= ~EPOLLOUT;
            m_pEpoll->Mod(m_fd,&m_ev);
        }
    }
}
//=================================================