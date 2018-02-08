#ifndef __terminal_cpp__
#define __terminal_cpp__

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "terminal.hpp"


terminal* terminal::instance = nullptr;


void terminal::init( int& sockud, int& sockue, int& socktc ){ //配置socket的属性
    //设置非阻塞模式
    int flags = fcntl(sockud, F_GETFL, 0);
    fcntl(sockud, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(sockue, F_GETFL, 0);
    fcntl(sockue, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(socktc, F_GETFL, 0);
    fcntl(socktc, F_SETFL, flags | O_NONBLOCK);
    //绑定地址
    sockaddr_in hostaddr;
    memset( &hostaddr, 0, sizeof(hostaddr) );
    hostaddr.sin_addr.s_addr = INADDR_ANY;
    hostaddr.sin_family = AF_INET;

    //TODO 错误处理

//    /*echo*/hostaddr.sin_port = htons(terminal_echo);bind( sockue, (sockaddr*)&hostaddr, sizeof(sockaddr) );
//    /*discover*/hostaddr.sin_port = htons(controller_discover);bind( sockud, (sockaddr*)&hostaddr, sizeof(sockaddr) );
//    /*control*/hostaddr.sin_port = htons(terminal_ctl);bind( socktc, (sockaddr*)&hostaddr, sizeof(sockaddr*) );
//    listen( socktc, 2 );
    /*echo*/hostaddr.sin_port = htons(3011);bind( sockue, (sockaddr*)&hostaddr, sizeof(sockaddr) );
    /*discover*/hostaddr.sin_port = htons(3012);bind( sockud, (sockaddr*)&hostaddr, sizeof(sockaddr) );
    /*control*/hostaddr.sin_port = htons(3013);bind( socktc, (sockaddr*)&hostaddr, sizeof(sockaddr) );
    listen( socktc, 2 );
}

/**
 * 用于处理controller的回声反馈的方法
 */
void terminal::echo( const stringx& pack ){
    try {
        stringx data;
        char cmd = package::unpack(pack, data );
        if( cmd == cmd_echo ) {
            secho = 0;//复位回声标志
            std::cout << "收到心跳确认,心跳标志复位" << std::endl;
        }

    }catch( const char* e ){
        //TODO Log here
        std::cout << "心跳确认包错误: " << e << std::endl;
        //throw e;
    }
}   


void terminal::discover( const stringx& pack, in_addr addr ) {//处理controller的发现响应
    try {
        stringx data;
        char cmd = package::unpack( pack, data );
        if( cmd == cmd_cavl ) {
            ctladdr.s_addr = addr.s_addr;
            ctlonline = true;
            sreq = 0;
            secho = 0;
            std::cout << "收到控制器有效包,控制器上线" << std::endl;
        }
    }catch( const char* e ) {
        //TODO Log here
        std::cout << "控制器有效包错误: " << e << std::endl;
        //throw e;
    }

}

void* terminal::callback( void* p ) {
    int sock = *((int*)p);
    *((int*)p) = 0;

    char rcvbuf[512];
    stringx rcvpack;
    int ret;
    sockaddr_in rcvaddr;
    socklen_t addrlen = sizeof(sockaddr);

    //接收数据
    std::cout << "开始接收指令 : ";
    do {
        ret = recv( sock, rcvbuf, 512, 0 );
        if( ret > 0 ) rcvpack += stringx(rcvbuf, ret );
    }while( ret != 0 );

    try{
        stringx data;
        int id;
        time_t  tt = time(NULL);
        char cmd = package::gateunpack( rcvpack, data ,&id);
        switch( cmd ) {
            case cmd_tctl:  //闸机控制指令
                std::cout << "闸机控制" << std::endl;
                instance->fgate( data[0] == 1 && id!= instance->userid && (tt-instance->pre_time)>5);
                instance->userid = id;
                instance->sreq = 0;   //复位请求
                break;
        }
    }catch( const char* e ) {
        std::cout << "指令错误: " << e << std::endl;
        //throw e;
    }

    std::cout << std::endl;
    close(sock);
}

/**
 * 回声-回调线程
 */
void* terminal::echo_callback( void* p ) {

    std::cout << "echo-callback线程开始运行" << std::endl;
    //注册心跳定时任务(同时也是控制器发现定时任务)
    signal( SIGALRM, []( int sig ){

        if( sig != SIGALRM or instance == nullptr ) return;

        sockaddr_in taga;
        memset( &taga, 0, sizeof(taga) );
        taga.sin_family = AF_INET;
        taga.sin_addr = instance->ctladdr;

        int sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if( sockfd == -1 ) {
            //TODO log a fail
            throw "failed to create socket for echo";
        }

        //判断控制器是否在线以决定动作
        if( instance->ctlonline ) {
            if( instance->secho > 0 ) { //检查心跳标志
                std::cout << "心跳确认超时,进入控制器发现模式" << std::endl;
                instance->ctlonline = false;
                alarm(1);   //尽快进入控制器发现模式
            } else {    //发送心跳数据包
                stringx pack = package::packup( cmd_echo, instance->guid );
                taga.sin_port = htons(terminal_echo);
                if( sendto( sockfd, pack, pack.length(), 0, (sockaddr*)&taga, sizeof(sockaddr) ) <= 0 )
                    std::cout << "心跳包发送失败: " << errno << std::endl;
                else
                    std::cout << "发送心跳包" << std::endl;
                instance->secho = 1;  //设置心跳标志
                alarm(10);
            }
        } else { //控制器发现模式
            int on = 1;
            if( setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) ) {//打开广播功能
                std::cout << "设置广播权限失败" << std::endl;
            }
            taga.sin_port = htons(controller_discover);
            taga.sin_addr.s_addr = INADDR_BROADCAST;
            stringx pack = package::packup( cmd_cdis, instance->guid );
            if( sendto( sockfd, pack, pack.length(), 0, (sockaddr*)&taga, sizeof(sockaddr) ) <= 0 )
                std::cout << "广播控制器发现包失败: " << errno << std::endl;
            else
                std::cout << "广播控制器发现包" << std::endl;
            alarm(5);
        }
        close(sockfd);
        return;
    } );
    //立即启动定时任务
    alarm(1);

    //创建套接字
    int sockud = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );    //for udp discover
    int sockue = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );    //for udp echo
    int socktc = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );    //for tcp control
    instance->init( sockud, sockue, socktc );
    
    char *rcvbuf = new char[512];

    //接下来是callback逻辑
    while( true ) {

        if( instance == nullptr ) {
            close(sockud);
            close(sockue);
            close(socktc);
            delete[] rcvbuf;
            return 0;
        }

        if( instance->ctlonline ) { //如果控制器在线
            stringx rcvpack;
            int ret;
            sockaddr_in rcvaddr;
            socklen_t addrlen = sizeof(sockaddr);

            //接收数据
            do {
                ret = recvfrom( sockue, rcvbuf, 512, 0, (sockaddr*)&rcvaddr, &addrlen );
                if( ret > 0 ) rcvpack += stringx(rcvbuf, ret );
            }while( ret > 0 );

            if( rcvaddr.sin_addr.s_addr != instance->ctladdr.s_addr or rcvpack.length() <= 0 ) {
                //若数据来自其他IP则抛弃此包
            } else {
                instance->echo( rcvpack );
            }

            int sockcc = accept( socktc, nullptr, 0 );
            if( sockcc > 0 ){
                pthread_t t;
                pthread_create( &t, nullptr, callback, &sockcc );
                while( sockcc != 0 );
            }
        } else {
            stringx rcvpack;
            int ret;
            sockaddr_in rcvaddr;
            socklen_t addrlen = sizeof(sockaddr);

            //接收数据
            do {
                ret = recvfrom( sockud, rcvbuf, 512, 0, (sockaddr*)&rcvaddr, &addrlen );
                if( ret > 0 ) rcvpack += stringx(rcvbuf, ret );
            }while( ret > 0 );
            if( rcvpack.length() > 0 )
                instance->discover( rcvpack, rcvaddr.sin_addr );
        }

    }

    return 0;
}

terminal::terminal(const char* _guid ):
tid_ec(0),sreq(0),treq(0),ctlonline(false),secho(0),guid(_guid),pre_time(0){

    if( terminal::instance == nullptr )
        terminal::instance = this;
    else
        throw "you have already created one instance";
    std::cout << "创建terminal实例" << std::endl;
    if( pthread_create( &tid_ec, nullptr, echo_callback, this ) ){
        std::cout << "创建echo-callback线程失败" << std::endl;
    } else {
        std::cout << "创建echo-callback线程" << std::endl;
    }
    terminal::userid = -1;
    terminal::pre_time = time(NULL);
}

terminal::~terminal() {
    std::cout << "terminal实例销毁" << std::endl;
    instance = nullptr;
}

/**
 * 创建一个认证请求
 * 发送与否取决于当前情景
 * 
 * 参数:
 *  eformat 图像格式
 *  width   图像宽度
 *  height  图像高度
 *  facea   人脸角度
 *  data    图像数据
 * 返回值:
 *  告诉前端此请求是否会被发送
 */
bool terminal::request( eformat eformat, short width, short height, char facea, const stringx& data ) {
    //sreq == 0 and
    if( ctlonline  ) {
        char *pack = new char[6];
        pack[0] = (char)eformat;
        pack[1] = (width>>0)&0x00ff;
        pack[2] = (width>>8)&0x00ff;
        pack[3] = (height>>0)&0x00ff;
        pack[4] = (height>>8)&0x00ff;
        pack[5] = facea;
        stringx rdata = stringx( pack, 6 )+data;
        //准备数据包
        rdata = package::packup( cmd_areq, rdata );
        delete[] pack;

        //发送请求
        sockaddr_in target;
        int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        memset( &target, 0, sizeof(target) );
        target.sin_addr.s_addr = ctladdr.s_addr;
        target.sin_family = AF_INET;
        target.sin_port = htons(terminal_req);
        if( connect( sock, (sockaddr*)&target, sizeof(target) ) != 0 ) {
            close(sock);
            return false;
        }
        std::cout << "认证请求数据长度: " << data.length() << std::endl;
        int len = 0;
        int total = rdata.length();
        while( len < total ) {
           // std::cout << ".";
            int ret = send( sock, &rdata[0]+len, (total-len>1024)?1024:(total-len), 0 );
            if( ret <= 0 ) {
                std::cout << "发送错误" << errno << std::endl;
                break;
            } else {
                len += ret;
            }
        }
        //int len = send( sock, rdata, rdata.length(), 0 );
        std::cout << std::endl << "发送认证请求: " << len << '(' << rdata.length() << ") bytes" << std::endl;
        sreq = 1;   //设置
        close(sock);
        return true;
    }
    return false;
}

/**
 * 绑定一个在场验证回调函数
 */
void terminal::setfield( std::function<bool()> func ) {
    ffield = func;
}

/**
 * 绑定一个闸机控制回调函数
 */
void terminal::setgate( std::function<bool(bool)> func ) {
    fgate = func;
}

#endif