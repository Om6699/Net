#ifndef __SERVICETYPEHEAD_H__
#define __SERVICETYPEHEAD_H__
#define INVALID_FD -1 
#define INVALID_SESSIONID 0xFFFFFFFFFFFFFFFF
//=================================================
union SessionID
{
    struct
	{
        short index;                //会话索引
        short cycle;                //循环计数
        int   fd;                   //模块文件描述符
    } sct;
    unsigned long long sid;         //应用层描述符
};
#endif