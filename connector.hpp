#ifndef __connector__
#define __connector__

#if __cplusplus < 201402L
#error cpp版本必须高于c++14 请使用-std=gnu++14选项 
#endif

#include "stringx.hpp"

//定义各通信端口 格式: 主机身份$用途
enum eport {
    terminal_req = 3001,
    controller_discover,
    terminal_echo,
    terminal_ctl,
};

enum ecmd {
    cmd_echo = 0,   //心跳和心跳确认
    cmd_tctl,       //终端控制
    cmd_areq,       //认证请求
    cmd_cdis,       //控制器发现
    cmd_cavl,       //控制器有效
};


//定义通信数据包
class package {
    /**
     * start(2) EAH AEH
     * cmd(1) *
     * length(4) *
     * content(*) *
     * eop(3) EEH EEH EEH end of package
     */
    public:
        stringx static packup( const char cmd,const stringx& content );
        stringx static packup( const char cmd, ... );
        char static unpack(const stringx& pack, stringx& content );
        char static gateunpack(const stringx &pack, stringx &content, int *id);
};

#endif