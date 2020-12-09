#pragma once
#include <vector>
#include <sys/epoll.h>

#define INVALID_FD -1 
#define INVALID_SESSIONID 0xFFFFFFFFFFFFFFFF
//=================================================
struct SessionID{
    union{
        short index;      //会话索引
        short cycle;      //循环计数
        int fd;           //模块文件描述符
        long long sid;    //应用层描述符
    };
};
//=================================================
class IEpEvent{
    public:
        virtual void DoEpEvent(int evType) = 0; 
};
//=================================================
class NetServer;
class EpollIo {
    friend NetServer;
private:
    int m_maxEvent;
    int m_epfd;
public:
    EpollIo(int maxEvent);
    ~EpollIo();
public:
    int GetEpFd(){ return m_epfd; }
    int Mod(int fd,epoll_event* ev);
    int Add(int fd,epoll_event* ev);
    int Del(int fd);
protected:
    void Loop(int timeOut);
};
//=================================================
class IListeneSink {
    public:
        virtual void Listene(int fd,int addr) = 0;
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
    virtual void DoEpEvent(int evType); 
public:
    bool StartListen();
protected:
    bool SetNonBlock(int fd);
};
//=================================================
//=================================================
class Session : public IEpEvent {
    friend NetServer;
private:
    //
    short m_idex;
    short m_cycle;
    int m_fd;
    int m_addr;
    epoll_event m_ev;
    EpollIo* m_pEpoll;
    //
    int  m_size;
    char m_sendbuf[1024];
public:
    Session(short idex,EpollIo* pEpoll);
    ~Session();
public:
    virtual void DoEpEvent(int evType); 
public:
    void Attch(int fd,int addr,short cycle);
    void Reset();
    bool Send(void* buf,int size);
protected:
    void DoRecv();
    void DoSend();
};
//=================================================
typedef std::vector<Session*> SessionVec;
class NetServer : public IListeneSink {
    EpollIo    m_Io;
    Listener   m_pListener;
    SessionVec m_SessionVec;
    short      m_Cycle;
public:
    NetServer(short poolSize,int port);
    ~NetServer();
public:
    virtual void Listene(int fd,int addr);
public:
    void run(int timeOut = -1);
};
