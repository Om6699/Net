#include "NetService.h"
#include "Session.h"

NetService::NetService(short poolSize,int port):m_Io(poolSize),m_Listener(port,&m_Io,this),m_Buff(poolSize * SESSIONBUFFSIZE)
{
    for (short i = 0; i < poolSize; i++) {
        m_SessionVec.push_back(new Session(i,&m_Io,&m_Buff));
    }
    m_Cycle = 0;
}

NetService::~NetService() 
{

}

void NetService::ListeneSink(int fd,uint addr)
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
    m_Cycle = ((++m_Cycle) % INT16_MAX);
}

void NetService::run(int timeOut)
{
    m_Listener.StartListen();
    for(;;) {
        m_Io.Wait(timeOut);
    }
}


bool NetService::SendData(SessionID sid,void* pdata,int size)
{
    if(sid.Sun.index < 0 || sid.Sun.index >= (short)m_SessionVec.size())
        return false;
    Session* pSession = m_SessionVec[sid.Sun.index];
    if(!pSession) 
        return false;
    return pSession->Send(pdata,size);
}